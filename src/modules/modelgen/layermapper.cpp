#include "layermapper.h"

LayerMapper::LayerMapper() {
    Set2DNozzlePolygon(0.46f);
}

void LayerMapper::Set2DNozzlePolygon(float diameter)
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
    this->nozzle = nozzle;}

Polygon_2 LayerMapper::place_nozzle_at(Polygon_2 nozzle, Point_2 vertex) {
    Polygon_2 translated_nozzle;

    for (const auto &pt : nozzle.vertices()) {
        Point_2 translated_point = Point_2(pt.x() + vertex.x(), pt.y() + vertex.y());
        translated_nozzle.push_back(translated_point);
    }

    if (translated_nozzle.is_clockwise_oriented()) translated_nozzle.reverse_orientation();

    return translated_nozzle;
}

std::list<Polygon_with_holes_2> LayerMapper::GenerateLayerFromPaths(std::vector<GCodePoint> points, std::vector<GCodePath> paths)
{
    double thickness = nozzle.diameter;
    double half_w = thickness / 2.0;

    std::unordered_set<std::string> unique_points;
    std::vector<Polygon_2> polygons;
    int pi = 0;
    for (const auto &path : paths) {
        GCodePoint p1 = points[path.start];
        GCodePoint p2 = points[path.end];

        // Create path vector
        Vector_2 direction(Point_2(p1.x, p1.z), Point_2(p2.x, p2.z));
        
        // Zero-length Check
        if (direction.squared_length() == 0) continue;

        // Normal vector
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
        
        polygons.push_back(rect);
        pi++;

        if(unique_points.find(std::to_string(p1.x) + "," + std::to_string(p1.z)) == unique_points.end()) {
            Polygon_2 nozzle_start = place_nozzle_at(nozzle.polygon, Point_2(p1.x, p1.z));
            polygons.push_back(nozzle_start);
            pi++;
            unique_points.insert(std::to_string(p1.x) + "," + std::to_string(p1.z));
        }
        if(unique_points.find(std::to_string(p2.x) + "," + std::to_string(p2.z)) == unique_points.end()) {
            Polygon_2 nozzle_end =  place_nozzle_at(nozzle.polygon, Point_2(p2.x, p2.z));
            polygons.push_back(nozzle_end);
            pi++;
            unique_points.insert(std::to_string(p2.x) + "," + std::to_string(p2.z));
        }

    }

    printf("Generated %d polygons from paths and nozzles.\n", pi);
    printf("Total polygons count: %lu\n", polygons.size());

    Polygon_set_2 merger;
    merger.join(polygons.begin(), polygons.end());

    
    std::list<Polygon_with_holes_2> final_output;
    merger.polygons_with_holes(std::back_inserter(final_output));
    
    // Print details of merger
    printf("Generated layer with %lu polygons.\n", final_output.size());
    // for (const auto &pwh : final_output) {
    //     printf(" Polygon with %lu outer vertices and %lu holes.\n", pwh.outer_boundary().size(), std::distance(pwh.holes_begin(), pwh.holes_end()));
    // }

    return final_output;
}

Mesh LayerMapper::Extrude2DLayerTo3D(std::list<Polygon_with_holes_2> layer, float layer_height)
{
    
    Mesh flat_mesh;

    CDT cdt;
        
    for (const auto &pwh : layer) {
        cdt.insert_constraint(pwh.outer_boundary().vertices_begin(), 
                            pwh.outer_boundary().vertices_end(), true);
        
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

Mesh LayerMapper::MergeTwoMesh(Mesh m1, Mesh m2) {
    Mesh result;
    CGAL::Polygon_mesh_processing::corefine_and_compute_union(m1, m2, result);
    return result;
}

Mesh LayerMapper::UnionMergeLayersToModel(std::vector<Mesh> layers)
{
    if (layers.empty()) return Mesh();

    std::deque<Mesh> meshes;
    for (auto& l : layers) meshes.push_back(std::move(l));

    while (meshes.size() > 1) {
        std::vector<std::future<Mesh>> futures;
        
        while (meshes.size() >= 2) {
            Mesh m1 = std::move(meshes.front()); meshes.pop_front();
            Mesh m2 = std::move(meshes.front()); meshes.pop_front();
            
            futures.push_back(std::async(std::launch::async, MergeTwoMesh, 
                            std::move(m1), std::move(m2)));
        }

        for (auto& f : futures) {
            meshes.push_back(f.get());
        }
    }

    return std::move(meshes.front());
}

Mesh LayerMapper::RemeshModel(Mesh model, float target_edge_length)
{
    if(!CGAL::is_triangle_mesh(model)) {
        CGAL::Polygon_mesh_processing::triangulate_faces(model);
    }

    EIFMap eif = get(CGAL::edge_is_feature, model);
    CGAL::Polygon_mesh_processing::detect_sharp_edges(model, 45, eif);

    CGAL::Polygon_mesh_processing::isotropic_remeshing(
        model.faces(),
        target_edge_length,
        model,
        CGAL::Polygon_mesh_processing::parameters::edge_is_constrained_map(eif)
        .number_of_iterations(1)
    );

    printf("Remeshed model to target edge length %.4f (placeholder).\n", target_edge_length);

    return model;
}

Object LayerMapper::MeshToRenderObject(const Mesh mesh)
{
    Object obj;
    obj.drawMode = GL_TRIANGLES;
    obj.useIndices = true;
    obj.color = glm::vec4(0.2f, 0.7f, 0.3f, 1.0f);
    
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    std::map<Mesh::Vertex_index, uint32_t> vertex_index_map;
    uint32_t current_index = 0;
    
    
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
    
    printf("Converted mesh to render object with %u vertices and %u indices.\n",
            (uint32_t)(vertices.size() / 3),
            (uint32_t)indices.size());

    
    glGenVertexArrays(1, &obj.VAO);
    glBindVertexArray(obj.VAO);

    
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

    
    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * indices.size(), indices.data(), GL_STATIC_DRAW);

    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    return obj;
}

Nef_polyhedron LayerMapper::MeshToNef(const Mesh& m) {
    if (m.is_empty()) return Nef_polyhedron();
    
    if (!CGAL::is_closed(m)) {
        printf("Warning: Mesh is not closed. Nef conversion may fail.\n");
    }

    return Nef_polyhedron(m); 
}

Mesh LayerMapper::NefToMesh(const Nef_polyhedron& nef) {
    Mesh m;

    if (nef.is_simple()) { 
        CGAL::convert_nef_polyhedron_to_polygon_mesh(nef, m);
    }

    return m;
}

Mesh LayerMapper::NefMergeLayersToModel(std::vector<Mesh> layers)
{
    Nef_polyhedron merge_nef;
    int completed = 0;
    int total = layers.size();
    for (const auto& m : layers) {
        printf("Merging layer %d/%d into Nef model...\n", ++completed, total);
        Nef_polyhedron layer_nef = MeshToNef(m);

        if (merge_nef.is_empty()) {
            merge_nef = layer_nef;
        } else {
            merge_nef += layer_nef;
            merge_nef.regularization(); 
        }
    }
    merge_nef.regularization(); 
    Mesh final_result = NefToMesh(merge_nef);
    return final_result;
}


Object LayerMapper::CreateRenderObjectFromMesh(std::vector<GCodeLayer> layers)
{
    std::vector<Mesh> layer_meshes;

    int i = 0;
    for (const auto &layer : layers) {
        // Debug
        // if (i++  == 10) {
        //     break;
        // }
        std::list<Polygon_with_holes_2> layer_polygons = GenerateLayerFromPaths(layer.points, layer.paths);
        Mesh layer_mesh = Extrude2DLayerTo3D(layer_polygons, layer.layerHeight);
        ShiftLayer(layer_mesh, layer.layer, layer.layerHeight);
        //layer_mesh = RemeshModel(layer_mesh, 0.5f);
        layer_meshes.push_back(layer_mesh);
        printf("Processed layer at Z=%.2f with %lu paths into mesh with %u vertices and %u faces.\n",
                layer.layer,
                layer.paths.size(),
                layer_mesh.number_of_vertices(),
                layer_mesh.number_of_faces());
    }

    Mesh final_model;
    if(Nef_based) {
        final_model = NefMergeLayersToModel(layer_meshes);
    }else{
        final_model = UnionMergeLayersToModel(layer_meshes);
    }

    if(remesh_after_layers)
        final_model = RemeshModel(final_model, 0.1f);
    Object MeshRenderObject = MeshToRenderObject(final_model);
    return MeshRenderObject;
}

void LayerMapper::ShiftLayer(Mesh& extruded_layer, float layer_offset, float layer_height) {
    double z_offset = layer_offset + layer_height;
    printf("layer offset: %.4f, layer height: %.4f, total z offset: %.4f\n", layer_offset, layer_height, z_offset);
    
    CGAL::Aff_transformation_3<K> translation(CGAL::TRANSLATION, K::Vector_3(0, z_offset, 0));

    for (auto v : extruded_layer.vertices()) {
        extruded_layer.point(v) = translation.transform(extruded_layer.point(v));
    }
}