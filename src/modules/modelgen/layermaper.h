

#include <unordered_set>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Polygon_set_2.h>
#include <CGAL/create_straight_skeleton_2.h>
#include <CGAL/create_offset_polygons_2.h>
#include <CGAL/Polygon_mesh_processing/extrude.h>
#include <CGAL/Boolean_set_operations_2.h>

#include <CGAL/Surface_mesh.h>

#include <CGAL/Triangulation_face_base_with_info_2.h>
#include <CGAL/Polygon_mesh_processing/triangulate_faces.h>
#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/mark_domain_in_triangulation.h>

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
    static Nozzle2D Generate2DNozzlePolygon(float diameter,NozzleQuality quality)
    {
        int num_points = 8;
        switch (quality)
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

    static Polygon_2 place_nozzle_at(Polygon_2 nozzle, Point_2 vertex) {
        Polygon_2 translated_nozzle;

        for (const auto &pt : nozzle.vertices()) {
            Point_2 translated_point = Point_2(pt.x() + vertex.x(), pt.y() + vertex.y());
            translated_nozzle.push_back(translated_point);
        }


        if (translated_nozzle.is_clockwise_oriented()) translated_nozzle.reverse_orientation();
        
        /*
            printf("Placed nozzle at (%.2f, %.2f) with %zu points.\n", CGAL::to_double(vertex.x()), CGAL::to_double(vertex.y()), translated_nozzle.size());
            printf("Nozzle points:\n");
            for (const auto &pt : translated_nozzle.vertices()) {
                printf("  (%.2f, %.2f)\n", CGAL::to_double(pt.x()), CGAL::to_double(pt.y()));
            } 
        */

        return translated_nozzle;
    }

    static std::list<Polygon_with_holes_2> GenerateLayerFromPaths(Nozzle2D nozzle, std::vector<GCodePoint> points, std::vector<GCodePath> paths)
    {
        double thickness = nozzle.diameter; // Example thickness for the extrusion
        Polygon_set_2 merger;
        double half_w = thickness / 2.0;

        std::unordered_set<std::string> unique_points;

        for (const auto &path : paths) {
            GCodePoint p1 = points[path.start];
            GCodePoint p2 = points[path.end];

            // 1. Calculate the direction vector and the perpendicular (normal) vector
            Vector_2 direction(Point_2(p1.x, p1.y), Point_2(p2.x, p2.y));
            
            // Skip zero-length segments
            if (direction.squared_length() == 0) continue;

            // Calculate unit normal vector
            double len = std::sqrt(CGAL::to_double(direction.squared_length()));
            Vector_2 normal(-direction.y() * (half_w / len), direction.x() * (half_w / len));

            Polygon_2 rect;
            Point_2 a(p1.x + CGAL::to_double(normal.x()), p1.y + CGAL::to_double(normal.y()));
            Point_2 b(p2.x + CGAL::to_double(normal.x()), p2.y + CGAL::to_double(normal.y()));
            Point_2 c(p2.x - CGAL::to_double(normal.x()), p2.y - CGAL::to_double(normal.y()));
            Point_2 d(p1.x - CGAL::to_double(normal.x()), p1.y - CGAL::to_double(normal.y()));

            rect.push_back(a);
            rect.push_back(b);
            rect.push_back(c);
            rect.push_back(d);

            // Ensure the polygon is oriented counter-clockwise for the Polygon_set_2
            if (rect.is_clockwise_oriented()) rect.reverse_orientation();
            
            merger.join(rect);

            if(unique_points.find(std::to_string(p1.x) + "," + std::to_string(p1.y)) == unique_points.end()) {
                Polygon_2 nozzle_start = place_nozzle_at(nozzle.polygon, Point_2(p1.x, p1.y));
                merger.join(nozzle_start);
                unique_points.insert(std::to_string(p1.x) + "," + std::to_string(p1.y));
            }
            if(unique_points.find(std::to_string(p2.x) + "," + std::to_string(p2.y)) == unique_points.end()) {
                Polygon_2 nozzle_end =  place_nozzle_at(nozzle.polygon, Point_2(p2.x, p2.y));
                merger.join(nozzle_end);
                unique_points.insert(std::to_string(p2.x) + "," + std::to_string(p2.y));
            }

        }

        printf("Merging polygons.\n");

        // 4. Extract result
        std::list<Polygon_with_holes_2> final_output;
        merger.polygons_with_holes(std::back_inserter(final_output));

        printf("Generated layer with %zu polygons.\n", final_output.size());
        for (const auto &pwh : final_output) {
            printf("Polygon with %zu outer vertices and %zu holes.\n", pwh.outer_boundary().size(), std::distance(pwh.holes_begin(), pwh.holes_end()));
            for (const auto &pt : pwh.outer_boundary().vertices()) {
                printf("  (%.2f, %.2f)\n", CGAL::to_double(pt.x()), CGAL::to_double(pt.y()));
            }
        }

        return final_output;
    }

    static void Extrude2DLayerTo3D(std::list<Polygon_with_holes_2> layer, float layer_height)
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
        
        CGAL::mark_domain_in_triangulation(cdt, in_domain);

        std::map<CDT::Vertex_handle, Mesh::Vertex_index> v_map;

        for(auto f : cdt.finite_face_handles()) {
            if (get(in_domain, f)) {
                Mesh::Vertex_index vi[3];
                for(int i=0; i<3; ++i) {
                    auto vh = f->vertex(i);
                    if (v_map.find(vh) == v_map.end()) {
                        Point_2 p2 = vh->point();
                        v_map[vh] = flat_mesh.add_vertex(Point_3(p2.x(), p2.y(), 0));
                    }
                    vi[i] = v_map[vh];
                }
                flat_mesh.add_face(vi[0], vi[1], vi[2]);
            }
        }

        Mesh extruded_layer;

        CGAL::Polygon_mesh_processing::extrude_mesh(flat_mesh, extruded_layer, K::Vector_3(0, 0, layer_height));
        
        printf("Extruded layer to 3D mesh with %zu vertices and %zu faces.\n",
                extruded_layer.number_of_vertices(),
                extruded_layer.number_of_faces());

        // print mesh virable as obj file to console
        for (const auto &v : extruded_layer.vertices()) {
            Point_3 p = extruded_layer.point(v);
            printf("v %.4f %.4f %.4f\n", CGAL::to_double(p.x()), CGAL::to_double(p.y()), CGAL::to_double(p.z()));
        }
        for (const auto &f : extruded_layer.faces()) {
            std::vector<Mesh::Vertex_index> face_vertices;
            for (const auto &v : CGAL::vertices_around_face(extruded_layer.halfedge(f), extruded_layer)) {
                face_vertices.push_back(v);
            }
            printf("f");
            for (const auto &v : face_vertices) {
                printf(" %zu", static_cast<size_t>(v) + 1);
            }
            printf("\n");
        }
    }
};
