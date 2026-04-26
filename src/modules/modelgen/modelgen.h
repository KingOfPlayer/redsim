#pragma once
#include <unordered_set>

// General headers + 3D meshing 
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
#include <CGAL/Polygon_mesh_processing/detect_features.h>

#include <CGAL/Polyhedron_3.h>
#include <CGAL/Nef_polyhedron_3.h>
#include <CGAL/boost/graph/convert_nef_polyhedron_to_polygon_mesh.h>

#include <boost/property_map/property_map.hpp>

typedef CGAL::Exact_predicates_exact_constructions_kernel K;
//typedef CGAL::Exact_predicates_inexact_constructions_kernel   K;
typedef K::Point_2 Point_2;
typedef K::Vector_2                                       Vector_2;
typedef K::Point_3                                        Point_3;
typedef K::Aff_transformation_2                           Transformation;
typedef CGAL::Polygon_2<K> Polygon_2;
typedef CGAL::Straight_skeleton_2<K> Straight_skeleton_2;
typedef CGAL::Polygon_with_holes_2<K>         Polygon_with_holes_2;
typedef CGAL::Polygon_set_2<K>        Polygon_set_2;
typedef CGAL::Surface_mesh<Point_3>                       Mesh;
typedef CGAL::Polyhedron_3<K> Polyhedron;
typedef CGAL::Nef_polyhedron_3<K> Nef_polyhedron;

struct FaceInfo2 {};

typedef CGAL::Triangulation_vertex_base_2<K>             Vb;
typedef CGAL::Triangulation_face_base_with_info_2<FaceInfo2, K> Fb;
typedef CGAL::Constrained_triangulation_face_base_2<K, Fb>      CFb;
typedef CGAL::Triangulation_data_structure_2<Vb, CFb>           TDS;
typedef CGAL::Exact_intersections_tag                           Itag;
typedef CGAL::Constrained_Delaunay_triangulation_2<K, TDS, Itag> CDT;

typedef CDT::Face_handle                                          Face_handle;
typedef boost::property_map<Mesh, CGAL::edge_is_feature_t>::type EIFMap;

#include <CGAL/Polyhedral_mesh_domain_with_features_3.h>
#include <CGAL/Mesh_triangulation_3.h>
#include <CGAL/Mesh_complex_3_in_triangulation_3.h>
#include <CGAL/Mesh_criteria_3.h>
#include <CGAL/make_mesh_3.h>

#include <CGAL/tetrahedral_remeshing.h>