#include "tetrahedralmesher.h"

#include <CGAL/make_mesh_3.h>
#include <CGAL/tetrahedral_remeshing.h>
#include <CGAL/Orthogonal_k_neighbor_search.h>
#include <CGAL/Search_traits_3.h>


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
}

C3t3 TetrahedralMesher::LabelC3t3(C3t3& c3t3, const std::vector<LabeledVertexGroup>& labeledVertexGroups) 
{

    for (auto fit = c3t3.facets_in_complex_begin(); fit != c3t3.facets_in_complex_end(); ++fit) 
        {
            c3t3.set_surface_patch_index(*fit, 0);
        }

    double max_distance_allowed = cell_size * 1.2;
    double max_sq_distance = max_distance_allowed * max_distance_allowed;

    for (const auto& labeledVertexGroup : labeledVertexGroups) {

        std::vector<Point_3_fast> target_points;
        target_points.reserve(labeledVertexGroup.points.size());
        for (const auto& v : labeledVertexGroup.points) {
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

            if (c3t3.surface_patch_index(*fit) != 0)
                continue;

            Cell_handle cell    = fit->first;
            int         oppIdx  = fit->second;

            auto p0 = cell->vertex((oppIdx + 1) % 4)->point().point();
            auto p1 = cell->vertex((oppIdx + 2) % 4)->point().point();
            auto p2 = cell->vertex((oppIdx + 3) % 4)->point().point();

            Point_3_fast centroid = CGAL::centroid(p0, p1, p2);

            Neighbor_search search(tree, centroid, 1);
            double sq_dist = search.begin()->second;

            if (sq_dist < max_sq_distance) {
                c3t3.set_surface_patch_index(*fit, labeledVertexGroup.label);
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
/*Mesh TetrahedralMesher::TetrahedralToMesh(const Triangulation_3& tr) {
    Mesh output_mesh;

    for (auto fit = tr.finite_facets_begin(); fit != tr.finite_facets_end(); ++fit)
    {
        auto cell   = fit->first;
        int  index  = fit->second;
        auto neighbor = cell->neighbor(index);

        bool cell_inside     = (cell->subdomain_index()     != 0);
        bool neighbor_inside = (neighbor->subdomain_index() != 0);

        if (cell_inside != neighbor_inside)
        {
            std::array<Point_3, 3> pts;
            int k = 0;
            for (int i = 0; i < 4; ++i)
                if (i != index)
                    pts[k++] = cell->vertex(i)->point().point();

            auto v0 = output_mesh.add_vertex(pts[0]);
            auto v1 = output_mesh.add_vertex(pts[1]);
            auto v2 = output_mesh.add_vertex(pts[2]);
            output_mesh.add_face(v0, v1, v2);
        }
    }

    return output_mesh;
}*/

void TetrahedralMesher::SaveTetrahedralMesherResultToFile(const TetrahedralMesherResult& result, const std::string& filename) {
	std::ofstream out(filename);
	CGAL::IO::write_MEDIT(out, result.c3t3);
}

TetrahedralMesherResult TetrahedralMesher::ProcessMeshForTetrahedral(const Mesh& input_mesh) {
	TetrahedralMesherResult result;
	result.c3t3 = MeshToC3t3(input_mesh);
	// result.surface = TetrahedralToMesh(result.volume);
	return result;
}