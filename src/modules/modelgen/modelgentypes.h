#pragma once
#define CGAL_LINKED_WITH_TBB

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>

using K = CGAL::Exact_predicates_exact_constructions_kernel;
using Point_2 = K::Point_2;
using Vector_2 = K::Vector_2;
using Point_3 = K::Point_3;
using Mesh = CGAL::Surface_mesh<K::Point_3>;


using K_fast = CGAL::Exact_predicates_inexact_constructions_kernel;
using Point_2_fast = K_fast::Point_2;
using Vector_2_fast = K_fast::Vector_2;
using Point_3_fast = K_fast::Point_3;
using Mesh_fast = CGAL::Surface_mesh<K_fast::Point_3>;

