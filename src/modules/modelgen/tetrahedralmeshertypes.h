#pragma once

#include "modelgentypes.h"

#include <CGAL/Polyhedral_mesh_domain_with_features_3.h>
#include <CGAL/Mesh_triangulation_3.h>
#include <CGAL/Mesh_complex_3_in_triangulation_3.h>
#include <CGAL/Mesh_criteria_3.h>

#ifdef CGAL_CONCURRENT_MESH_3
using Concurrency_tag = CGAL::Parallel_if_available_tag;
#else
using Concurrency_tag = CGAL::Sequential_tag;
#endif

using Mesh_domain_fast  = CGAL::Polyhedral_mesh_domain_with_features_3<K_fast, Mesh_fast>;

using Tr           = CGAL::Mesh_triangulation_3<Mesh_domain_fast, CGAL::Default, Concurrency_tag>::type;
using C3t3         = CGAL::Mesh_complex_3_in_triangulation_3<Tr, Mesh_domain_fast::Corner_index, Mesh_domain_fast::Curve_index>;

using Mesh_criteria = CGAL::Mesh_criteria_3<Tr>;
using Triangulation_3 = CGAL::Triangulation_3<Tr::Geom_traits, Tr::Triangulation_data_structure>;
using Cell_handle = Triangulation_3::Cell_handle ;

using TreeTraits = CGAL::Search_traits_3<K_fast>; 
using Neighbor_search = CGAL::Orthogonal_k_neighbor_search<TreeTraits>;
using Tree = Neighbor_search::Tree;