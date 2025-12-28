

#include <unordered_set>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Polygon_set_2.h>
#include <CGAL/create_straight_skeleton_2.h>
#include <CGAL/create_offset_polygons_2.h>
#include <CGAL/Polygon_mesh_processing/extrude.h>
#include <CGAL/Polygon_mesh_processing/corefinement.h>
#include <CGAL/Boolean_set_operations_2.h>

#include <CGAL/Surface_mesh.h>

#include <CGAL/Triangulation_face_base_with_info_2.h>
#include <CGAL/Polygon_mesh_processing/triangulate_faces.h>
#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/mark_domain_in_triangulation.h>
#include <CGAL/Polygon_mesh_processing/remesh.h>

#include <boost/property_map/property_map.hpp>

#include "../gcode/gcode.h"
#pragma once

typedef CGAL::Exact_predicates_exact_constructions_kernel K;
typedef K::Point_2 Point_2;
typedef K::Vector_2                                       Vector_2;
typedef K::Point_3                                        Point_3;
typedef K::Aff_transformation_2                           Transformation;
typedef CGAL::Polygon_2<K> Polygon_2;
typedef CGAL::Straight_skeleton_2<K> Straight_skeleton_2;
typedef CGAL::Polygon_with_holes_2<K>         Polygon_with_holes_2;
typedef CGAL::Polygon_set_2<K>        Polygon_set_2;
typedef CGAL::Surface_mesh<Point_3>                       Mesh;


struct FaceInfo2 {
    bool nesting_level;
    bool in_domain() { return nesting_level % 2 == 1; }
};

typedef CGAL::Triangulation_vertex_base_2<K>             Vb;
typedef CGAL::Triangulation_face_base_with_info_2<FaceInfo2, K> Fb;
typedef CGAL::Constrained_triangulation_face_base_2<K, Fb>      CFb;
typedef CGAL::Triangulation_data_structure_2<Vb, CFb>           TDS;
typedef CGAL::Exact_intersections_tag                           Itag;
typedef CGAL::Constrained_Delaunay_triangulation_2<K, TDS, Itag> CDT;

typedef CDT::Face_handle                                          Face_handle;

struct Nozzle2D {
    Polygon_2 polygon;
    float diameter;
};

class LayerMapper
{
public:
    enum NozzleQuality
    {
        LOW,
        MEDIUM,
        HIGH
    }; 
NozzleQuality nozzleQuality = MEDIUM;
Nozzle2D nozzle;

    LayerMapper() {
        nozzle = Generate2DNozzlePolygon(0.4f);
    }

    Nozzle2D Generate2DNozzlePolygon(float diameter)
    {
        int num_points = 8;
        switch (nozzleQuality)
        {
        case LOW:
            num_points = 8;
            break;
        case MEDIUM:
            num_points = 16;
            break;
        case HIGH:
            num_points = 32;
            break;
        }

        Polygon_2 polygon;
        for (int i = 0; i < num_points; ++i)
        {
            double angle = 2.0 * CGAL_PI * i / num_points;
            double x = (diameter / 2.0) * cos(angle);
            double y = (diameter / 2.0) * sin(angle);
            polygon.push_back(Point_2(x, y));
        }

        Nozzle2D nozzle;
        nozzle.polygon = polygon;
        nozzle.diameter = diameter;
        return nozzle;
    }

    Polygon_2 place_nozzle_at(Polygon_2 nozzle, Point_2 vertex) {
        Polygon_2 translated_nozzle;

        for (const auto &pt : nozzle.vertices()) {
            Point_2 translated_point = Point_2(pt.x() + vertex.x(), pt.y() + vertex.y());
            translated_nozzle.push_back(translated_point);
        }


        if (translated_nozzle.is_clockwise_oriented()) translated_nozzle.reverse_orientation();
        
        /*
            printf("Placed nozzle at (%.2f, %.2f) with %lu points.\n", CGAL::to_double(vertex.x()), CGAL::to_double(vertex.y()), translated_nozzle.size());
            printf("Nozzle points:\n");
            for (const auto &pt : translated_nozzle.vertices()) {
                printf("  (%.2f, %.2f)\n", CGAL::to_double(pt.x()), CGAL::to_double(pt.y()));
            } 
        */

        return translated_nozzle;
    }

    std::list<Polygon_with_holes_2> GenerateLayerFromPaths(std::vector<GCodePoint> points, std::vector<GCodePath> paths)
    {
        double thickness = nozzle.diameter; // Example thickness for the extrusion
        Polygon_set_2 merger;
        double half_w = thickness / 2.0;

        std::unordered_set<std::string> unique_points;

        for (const auto &path : paths) {
            GCodePoint p1 = points[path.start];
            GCodePoint p2 = points[path.end];

            // 1. Calculate the direction vector and the perpendicular (normal) vector
            Vector_2 direction(Point_2(p1.x, p1.z), Point_2(p2.x, p2.z));
            
            // Skip zero-length segments
            if (direction.squared_length() == 0) continue;

            // Calculate unit normal vector
            double len = std::sqrt(CGAL::to_double(direction.squared_length()));
            Vector_2 normal(-direction.y() * (half_w / len), direction.x() * (half_w / len));

            Polygon_2 rect;
            Point_2 a(p1.x + CGAL::to_double(normal.x()), p1.z + CGAL::to_double(normal.y()));
            Point_2 b(p2.x + CGAL::to_double(normal.x()), p2.z + CGAL::to_double(normal.y()));
            Point_2 c(p2.x - CGAL::to_double(normal.x()), p2.z - CGAL::to_double(normal.y()));
            Point_2 d(p1.x - CGAL::to_double(normal.x()), p1.z - CGAL::to_double(normal.y()));

            rect.push_back(a);
            rect.push_back(b);
            rect.push_back(c);
            rect.push_back(d);

            // Ensure the polygon is oriented counter-clockwise for the Polygon_set_2
            if (rect.is_clockwise_oriented()) rect.reverse_orientation();
            
            merger.join(rect);

            if(unique_points.find(std::to_string(p1.x) + "," + std::to_string(p1.z)) == unique_points.end()) {
                Polygon_2 nozzle_start = place_nozzle_at(nozzle.polygon, Point_2(p1.x, p1.z));
                merger.join(nozzle_start);
                unique_points.insert(std::to_string(p1.x) + "," + std::to_string(p1.z));
            }
            if(unique_points.find(std::to_string(p2.x) + "," + std::to_string(p2.z)) == unique_points.end()) {
                Polygon_2 nozzle_end =  place_nozzle_at(nozzle.polygon, Point_2(p2.x, p2.z));
                merger.join(nozzle_end);
                unique_points.insert(std::to_string(p2.x) + "," + std::to_string(p2.z));
            }

        }

        // 4. Extract result
        std::list<Polygon_with_holes_2> final_output;
        merger.polygons_with_holes(std::back_inserter(final_output));
        
        // print details of merger
        

        printf("Generated layer with %lu polygons.\n", final_output.size());
        // for (const auto &pwh : final_output) {
        //     printf(" Polygon with %lu outer vertices and %lu holes.\n", pwh.outer_boundary().size(), std::distance(pwh.holes_begin(), pwh.holes_end()));
        // }

        return final_output;
    }

    Mesh Extrude2DLayerTo3D(std::list<Polygon_with_holes_2> layer, float layer_height)
    {
        
        Mesh flat_mesh;

        CDT cdt;
            
            // 1. Insert outer boundary
        for (const auto &pwh : layer) {
            cdt.insert_constraint(pwh.outer_boundary().vertices_begin(), 
                                pwh.outer_boundary().vertices_end(), true);
            
            // 2. Insert holes
            for (auto i = pwh.holes_begin(); i != pwh.holes_end(); ++i) {
                cdt.insert_constraint(i->vertices_begin(), i->vertices_end(), true);
            }
        }
            std::unordered_map<Face_handle, bool> in_domain_map;
            boost::associative_property_map< std::unordered_map<Face_handle,bool> >
                in_domain(in_domain_map);
        
        printf("Triangulating layer.\n");

        // 3. Mark facets that are inside the domain
        CGAL::mark_domain_in_triangulation(cdt, in_domain);

        std::map<CDT::Vertex_handle, Mesh::Vertex_index> v_map;

        for(auto f : cdt.finite_face_handles()) {
            if (get(in_domain, f)) {
                Mesh::Vertex_index vi[3];
                for(int i=0; i<3; ++i) {
                    auto vh = f->vertex(i);
                    if (v_map.find(vh) == v_map.end()) {
                        Point_2 p2 = vh->point();
                        v_map[vh] = flat_mesh.add_vertex(Point_3(p2.x(), 0, p2.y()));
                    }
                    vi[i] = v_map[vh];
                }
                flat_mesh.add_face(vi[0], vi[1], vi[2]);
            }
        }

        Mesh extruded_layer;

        CGAL::Polygon_mesh_processing::extrude_mesh(flat_mesh, extruded_layer, K::Vector_3(0, layer_height, 0));
        
        printf("Extruded layer to 3D mesh with %u vertices and %u faces.\n",
                extruded_layer.number_of_vertices(),
                extruded_layer.number_of_faces());

        return extruded_layer;
    }

    Mesh MergeLayersToModel(std::vector<Mesh> layers)
    {
        Mesh final_model;

        for (auto &layer : layers) {
            CGAL::Polygon_mesh_processing::corefine_and_compute_union(final_model, layer, final_model);
        }

        printf("Merged %lu layers into final model with %u vertices and %u faces.\n",
                layers.size(),
                final_model.number_of_vertices(),
                final_model.number_of_faces());

        return final_model;
    }

    Mesh RemeshModel(const Mesh model, float target_edge_length)
    {
        Mesh remeshed_model = model;

        CGAL::Polygon_mesh_processing::isotropic_remeshing(
            faces(remeshed_model),
            target_edge_length,
            remeshed_model,
            CGAL::Polygon_mesh_processing::parameters::number_of_iterations(3)
        );

        printf("Remeshed model to target edge length %.4f (placeholder).\n", target_edge_length);

        return remeshed_model;
    }

    Object MeshToRenderObject(const Mesh mesh)
    {
        Object obj;
        obj.drawMode = GL_TRIANGLES;
        obj.useIndices = true;
        obj.color = glm::vec4(0.2f, 0.7f, 0.3f, 1.0f); // Greenish
        
        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        std::map<Mesh::Vertex_index, uint32_t> vertex_index_map;
        uint32_t current_index = 0;
        
        // move to origin
        glm::vec3 min(1e9), max(-1e9);
        for (const auto &v : mesh.vertices()) {
            Point_3 p = mesh.point(v);
            if(CGAL::to_double(p.x()) < min.x) min.x = CGAL::to_double(p.x());
            if(CGAL::to_double(p.x()) > max.x) max.x = CGAL::to_double(p.x());
            if(CGAL::to_double(p.y()) < min.y) min.y = CGAL::to_double(p.y());
            if(CGAL::to_double(p.y()) > max.y) max.y = CGAL::to_double(p.y());
            if(CGAL::to_double(p.z()) < min.z) min.z = CGAL::to_double(p.z());
            if(CGAL::to_double(p.z()) > max.z) max.z = CGAL::to_double(p.z());
        }

        glm::vec3 center = (min + max) * 0.5f;

        for (const auto &v : mesh.vertices()) {
            Point_3 p = mesh.point(v);
            vertices.push_back(CGAL::to_double(p.x()) - center.x);
            vertices.push_back(CGAL::to_double(p.y()) - center.y);
            vertices.push_back(CGAL::to_double(p.z()) - center.z);
            vertex_index_map[v] = current_index++;
        }

        for (const auto &f : mesh.faces()) {
            std::vector<Mesh::Vertex_index> face_vertices;
            for (const auto &v : CGAL::vertices_around_face(mesh.halfedge(f), mesh)) {
                face_vertices.push_back(v);
            }
            if (face_vertices.size() == 3) {
                indices.push_back(vertex_index_map[face_vertices[0]]);
                indices.push_back(vertex_index_map[face_vertices[1]]);
                indices.push_back(vertex_index_map[face_vertices[2]]);
            }
        }

        obj.vertexCount = static_cast<uint32_t>(indices.size());

            // 3. Generate and Bind VAO
        glGenVertexArrays(1, &obj.VAO);
        glBindVertexArray(obj.VAO);

        // 4. Create VBO (Vertex Data)
        GLuint vbo;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

        // 5. Create EBO (Index Data)
        GLuint ebo;
        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * indices.size(), indices.data(), GL_STATIC_DRAW);

        // 6. Set Attribute Protocol (Location 0 = Position)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Unbind to stay clean
        glBindVertexArray(0);

        return obj;
    }

    Object CreateRenderObjectFromMesh(std::vector<GCodeLayer> layers)
    {
        std::vector<Mesh> layer_meshes;

        // Test first layer only
        int i = 0;
        for (const auto &layer : layers) {
            if (i++ == 5) break;
            std::list<Polygon_with_holes_2> layer_polygons = GenerateLayerFromPaths(layer.points, layer.paths);
            Mesh layer_mesh = Extrude2DLayerTo3D(layer_polygons, layer.layerHeight); // Example layer height
            //layer_mesh = RemeshModel(layer_mesh, 0.5f); // Example target edge length
            layer_meshes.push_back(layer_mesh);
            printf("Processed layer at Z=%.2f with %lu paths into mesh with %u vertices and %u faces.\n",
                    layer.layer,
                    layer.paths.size(),
                    layer_mesh.number_of_vertices(),
                    layer_mesh.number_of_faces());
        }

        Mesh final_model = MergeLayersToModel(layer_meshes);
        //final_model = RemeshModel(final_model, 0.005f); // Example target edge length
        Object MeshRenderObject = MeshToRenderObject(final_model);
        return MeshRenderObject;
    }
};
