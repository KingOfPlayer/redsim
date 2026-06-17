#include "layermapper.h"

#include "../gcode/gcode.h"

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

std::vector<Polygon_with_holes_2> LayerMapper::GCodePathsToPolygons(std::vector<GCodePoint> points, std::vector<GCodePath> paths)
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

    /*
    printf("Generated %d polygons from paths and nozzles.\n", pi);
    printf("Total polygons count: %lu\n", polygons.size());
    */

    Polygon_set_2 merger;
    merger.join(polygons.begin(), polygons.end());

    
    std::vector<Polygon_with_holes_2> final_output;
    merger.polygons_with_holes(std::back_inserter(final_output));
    
    // Print details of merger
    //printf("Generated layer with %lu polygons.\n", final_output.size());
    // for (const auto &pwh : final_output) {
    //     printf(" Polygon with %lu outer vertices and %lu holes.\n", pwh.outer_boundary().size(), std::distance(pwh.holes_begin(), pwh.holes_end()));
    // }

    return final_output;
}

Mesh LayerMapper::PolygonsLayerToMesh(std::vector<Polygon_with_holes_2>& layer, float layer_height)
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

    CGAL::Polygon_mesh_processing::extrude_mesh(flat_mesh, extruded_layer, K::Vector_3(0, layer_height + LAYER_OVERLAP*2, 0));
    
    /*printf("Extruded layer to 3D mesh with %u vertices and %u faces.\n",
            extruded_layer.number_of_vertices(),
            extruded_layer.number_of_faces());*/

    return extruded_layer;
}

Mesh LayerMapper::MergeTwoMesh(Mesh m1, Mesh m2) {
    Mesh result;
    CGAL::Polygon_mesh_processing::corefine_and_compute_union(m1, m2, result);
    //CGAL::Polygon_mesh_processing::remove_isolated_vertices(result);
    return result;
}

Mesh LayerMapper::MergeLayersToModel(std::vector<Mesh> layers)
{
    auto start_time = std::chrono::high_resolution_clock::now();
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

    auto end_time = std::chrono::high_resolution_clock::now();
    long long elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    printf("Layer merging completed in %lld  millisecond.\nLayer speed: %.2f layers/millisecond.\n", elapsed, layers.size() / (elapsed / 1000.0));

    return std::move(meshes.front());
}

Mesh LayerMapper::RemeshModel(Mesh model)
{
    auto start_time = std::chrono::high_resolution_clock::now();
    if(!CGAL::is_triangle_mesh(model)) {
        CGAL::Polygon_mesh_processing::triangulate_faces(model);
    }

    EIFMap eif = get(CGAL::edge_is_feature, model);
    CGAL::Polygon_mesh_processing::detect_sharp_edges(model, remesh_edge_angle, eif);

    CGAL::Polygon_mesh_processing::isotropic_remeshing(
        model.faces(),
        remesh_target_length,
        model,
        CGAL::Polygon_mesh_processing::parameters::edge_is_constrained_map(eif)
        .number_of_iterations(remesh_iterations)
    );

    auto end_time = std::chrono::high_resolution_clock::now();
    long long elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    printf("Remeshing completed in %lld  millisecond.\n", elapsed);

    return model;
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

Mesh LayerMapper::MergeLayersToModelWithNef(std::vector<Mesh> layers)
{
    int total = layers.size();
    Nef_polyhedron final_merge_nef;
    
    int total_cores = std::thread::hardware_concurrency();
    omp_set_num_threads(total_cores);
    auto start_time = std::chrono::high_resolution_clock::now();
    #pragma omp parallel
    {
        Nef_polyhedron local_nef;

        #pragma omp for schedule(dynamic)
        for (int i = 0; i < total; ++i) {
            Nef_polyhedron local_layer = MeshToNef(layers[i]);

            if (local_nef.is_empty()) {
                local_nef = local_layer;
            } else {
                local_nef += local_layer;
            }
        }

        #pragma omp critical
        {
            if (final_merge_nef.is_empty()) {
                final_merge_nef = local_nef;
            } else if (!local_nef.is_empty()) {
                final_merge_nef += local_nef;
            }
        }
    }
    final_merge_nef.regularization(); 
    Mesh final_result = NefToMesh(final_merge_nef);

    auto end_time = std::chrono::high_resolution_clock::now();
    long long elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    printf("Nef merging completed in %lld  millisecond.\n", elapsed);

    return final_result;
}

std::vector<Mesh> LayerMapper::GCodeToMeshLayers(std::vector<GCodeLayer> layers)
{

    auto start_time = std::chrono::high_resolution_clock::now();
    std::vector<Mesh> layer_meshes(layers.size());

    int total_cores = std::thread::hardware_concurrency();
    omp_set_num_threads(total_cores);
    #pragma omp parallel for schedule(dynamic)
    for (size_t i = 0; i < layers.size(); ++i) {
        const auto& layer = layers[i];
        std::vector<Polygon_with_holes_2> layer_polygons =
            GCodePathsToPolygons(layer.points, layer.paths);

        Mesh layer_mesh = PolygonsLayerToMesh(layer_polygons, layer.layerHeight);
        LayerMapper::ShiftLayerMesh(layer_mesh, layer.layer, layer.layerHeight);

        layer_meshes[i] = std::move(layer_mesh);
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    long long elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    printf("Layer mesh generation and extrusion completed in %lld  millisecond.\nLayer speed: %.2f layers/millisecond.\n", elapsed, layers.size() / (elapsed / 1000.0));

    return layer_meshes;
}

Mesh LayerMapper::GenerateSurfaceMesh(std::vector<GCodeLayer> layers)
{
    std:: vector<Mesh> layer_meshes = GCodeToMeshLayers(layers);

    Mesh final_model;
    if(Nef_based) {
        final_model = MergeLayersToModelWithNef(layer_meshes);
    }else{
        final_model = MergeLayersToModel(layer_meshes);
    }
    
    if(remesh_after_layers){
        final_model = RemeshModel(final_model);
    }

    return final_model;
}

void LayerMapper::ShiftLayerMesh(Mesh& extruded_layer, float layer_offset, float layer_height) {
    double z_offset = layer_offset - layer_height + LAYER_OVERLAP;
    
    CGAL::Aff_transformation_3<K> translation(CGAL::TRANSLATION, K::Vector_3(0, z_offset, 0));

    for (auto v : extruded_layer.vertices()) {
        extruded_layer.point(v) = translation.transform(extruded_layer.point(v));
    }
}