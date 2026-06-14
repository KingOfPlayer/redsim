#include "tetrahedralmesher.h"

#include <CGAL/make_mesh_3.h>
#include <CGAL/tetrahedral_remeshing.h>
#include <CGAL/Orthogonal_k_neighbor_search.h>
#include <CGAL/Search_traits_3.h>

#include "../freefem/freefemtype.h"


C3t3 TetrahedralMesher::MeshToC3t3(
    const Mesh& input_mesh
)
{
    Mesh_fast input_mesh_fast = ModelgenHelper::MeshToMeshFast(input_mesh);
    
    if (CGAL::is_empty(input_mesh_fast))
        throw std::runtime_error("Input mesh is empty.");

    if (!CGAL::is_triangle_mesh(input_mesh_fast))
        throw std::runtime_error("Input mesh is not triangulated. Run your remesh step first.");
    
    Mesh_fast mesh_fast_copy = input_mesh_fast;

    double vol = CGAL::to_double(
        CGAL::Polygon_mesh_processing::volume(mesh_fast_copy)
    );
    std::cerr << "volume: " << vol << "\n";

    Mesh_domain_fast domain(mesh_fast_copy);

    domain.detect_features();

    Mesh_criteria criteria(
        CGAL::parameters::edge_size              = cell_size,
        CGAL::parameters::facet_angle            = facet_angle,
        CGAL::parameters::facet_size             = cell_size,
        CGAL::parameters::facet_distance         = facet_distance * cell_size,
        CGAL::parameters::cell_radius_edge_ratio = cell_radius_edge,
        CGAL::parameters::cell_size              = cell_size
    );

	// Prepare tetrahedral object
    C3t3 c3t3 = CGAL::make_mesh_3<C3t3>(
        domain, criteria,
        CGAL::parameters::no_perturb(),
        CGAL::parameters::no_exude()
    );

    if (c3t3.number_of_cells_in_complex() == 0)
        throw std::runtime_error("make_mesh_3 produced no tetrahedra. Check mesh is closed/manifold.");

    /*CGAL::refine_mesh_3(
        c3t3, domain, criteria,
        CGAL::parameters::no_perturb(),
        CGAL::parameters::no_exude()
    );*/

    // Tetrahedral mesh generation
    /*Triangulation_3 tr = CGAL::convert_to_triangulation_3(
        std::move(c3t3)
    );*/

    /*CGAL::tetrahedral_isotropic_remeshing(
        tr,
        cell_size,
        CGAL::parameters::number_of_iterations(remesh_iterations)
    );*/

    return c3t3;
}

C3t3 TetrahedralMesher::LabelC3t3(C3t3& c3t3, const std::vector<std::unique_ptr<VertexGroupBaseType>>& groups) 
{

    for (auto fit = c3t3.facets_in_complex_begin(); fit != c3t3.facets_in_complex_end(); ++fit) 
    {
        C3t3::Facet current_facet = *fit;
        c3t3.set_surface_patch_index(current_facet, 1);
    }

    double max_distance_allowed = cell_size * 1.2;
    double max_sq_distance = max_distance_allowed * max_distance_allowed;

    for (const auto& groupPtr : groups) {
        if (!groupPtr) continue;

        const auto& raw_points = groupPtr->getPoints();
        int current_label = groupPtr->getLabelID();

        std::vector<Point_3_fast> target_points;
        target_points.reserve(raw_points.size());
        for (const auto& v : raw_points) {
            target_points.emplace_back(
                CGAL::to_double(v.x),
                CGAL::to_double(v.y),
                CGAL::to_double(v.z)
            );
        }
        Tree tree(target_points.begin(), target_points.end());

        for (auto fit = c3t3.facets_in_complex_begin(); 
                  fit != c3t3.facets_in_complex_end(); ++fit) 
        {
            C3t3::Facet current_facet = *fit;
            if (c3t3.surface_patch_index(current_facet) > LABEL_ID_OFFSET)
                continue;

            Cell_handle cell = current_facet.first;
            int         index = current_facet.second;

            auto p0 = cell->vertex((index + 1) % 4)->point().point();
            auto p1 = cell->vertex((index + 2) % 4)->point().point();
            auto p2 = cell->vertex((index + 3) % 4)->point().point();

            Point_3_fast centroid = CGAL::centroid(p0, p1, p2);

            Neighbor_search search(tree, centroid, 1);
            double sq_dist = search.begin()->second;
            if (sq_dist < max_sq_distance) {
                c3t3.set_surface_patch_index(current_facet, current_label + LABEL_ID_OFFSET);

                printf("labeled from %d to %d\n", c3t3.surface_patch_index(current_facet), current_label + LABEL_ID_OFFSET);
            }
        }
    }
    return c3t3;
}

Triangulation_3 TetrahedralMesher::C3t3ToMesh(C3t3 c3t3)
{
    Triangulation_3 tr = CGAL::convert_to_triangulation_3(
        std::move(c3t3)
    );

    CGAL::tetrahedral_isotropic_remeshing(
        tr,
        cell_size,
        CGAL::parameters::number_of_iterations(remesh_iterations)
    );

    return tr;
}

// For future use
Mesh TetrahedralMesher::TetrahedralToMesh(const C3t3& c3t3) {
    Mesh output_mesh;
    std::map<C3t3::Triangulation::Vertex_handle, Mesh::Vertex_index> vertex_map;

    for (auto fit = c3t3.facets_in_complex_begin(); fit != c3t3.facets_in_complex_end(); ++fit)
    {
        Cell_handle cell = fit->first;
        int index        = fit->second;
        
        if (cell->subdomain_index() == 0) {
            cell = cell->neighbor(index);
            index = cell->index(fit->first);
        }
        int i1 = (index + 1) % 4;
        int i2 = (index + 2) % 4;
        int i3 = (index + 3) % 4;

        if (index % 2 == 0) {
            std::swap(i1, i2);
        }

        std::array<Cell_handle::value_type::Vertex_handle, 3> v_handles = {
            cell->vertex(i1),
            cell->vertex(i2),
            cell->vertex(i3)
        };

        std::array<Mesh::Vertex_index, 3> face_vertices;
        for (int i = 0; i < 3; ++i)
        {
            auto v_handle = v_handles[i];
            if (vertex_map.find(v_handle) == vertex_map.end())
            {
                auto pt_fast = v_handle->point().point();
                Mesh::Point pt(
                    CGAL::to_double(pt_fast.x()), 
                    CGAL::to_double(pt_fast.y()), 
                    CGAL::to_double(pt_fast.z())
                );
                vertex_map[v_handle] = output_mesh.add_vertex(pt);
            }
            face_vertices[i] = vertex_map[v_handle];
        }

        output_mesh.add_face(face_vertices[0], face_vertices[1], face_vertices[2]);
    }

    return output_mesh;
}

void TetrahedralMesher::SaveTetrahedralMesherResultToFile(const TetrahedralMesherResult& result, const std::string& filename) {
	std::ofstream out(filename);
	CGAL::IO::write_MEDIT(out, result.c3t3,
        CGAL::parameters::show_patches(true)
                     .rebind_labels(false)
                     .all_cells(true)
                     .all_vertices(true)
    );

    // Fix file 
    // First line "MeshVersionFormatted 1" change to "MeshVersionFormatted 0"
    std::fstream file(filename, std::ios::in | std::ios::out | std::ios::binary);
    if (!file.is_open()) return;

    std::string expectedHeader = "MeshVersionFormatted 1";
    std::string actualHeader(expectedHeader.length(), ' ');

    if (file.read(&actualHeader[0], expectedHeader.length())) {
        if (actualHeader == expectedHeader) {
            file.seekp(21, std::ios::beg);
            file.put('0'); 
        }
    }
}

TetrahedralMesherResult TetrahedralMesher::ProcessMeshForTetrahedral(const Mesh& input_mesh) {
	TetrahedralMesherResult result;
	result.c3t3 = MeshToC3t3(input_mesh);
	result.mesh = TetrahedralToMesh(result.c3t3);
	return result;
}