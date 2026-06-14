#pragma once

#include "modelgentypes.h"

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

// extra 
#include <CGAL/Polygon_mesh_processing/merge_border_vertices.h>
#include <CGAL/Polygon_mesh_processing/stitch_borders.h>
#include <CGAL/Polygon_mesh_processing/repair.h>

//#include <CGAL/property_map.h>
#include <boost/property_map/property_map.hpp>

using Transformation = K::Aff_transformation_2 ;
using Polygon_2 = CGAL::Polygon_2<K> ;
using Straight_skeleton_2 = CGAL::Straight_skeleton_2<K> ;
using Polygon_with_holes_2 = CGAL::Polygon_with_holes_2<K> ;
using Polygon_set_2 = CGAL::Polygon_set_2<K> ;
using Polyhedron = CGAL::Polyhedron_3<K> ;
using Nef_polyhedron = CGAL::Nef_polyhedron_3<K> ;

struct FaceInfo2
{
};

using Vb = CGAL::Triangulation_vertex_base_2<K>;
using Fb = CGAL::Triangulation_face_base_with_info_2<FaceInfo2, K>;
using CFb = CGAL::Constrained_triangulation_face_base_2<K, Fb>;
using TDS = CGAL::Triangulation_data_structure_2<Vb, CFb>;
using Itag = CGAL::Exact_intersections_tag;
using CDT = CGAL::Constrained_Delaunay_triangulation_2<K, TDS, Itag>;

using Face_handle = CDT::Face_handle;
using EIFMap = boost::property_map<Mesh, CGAL::edge_is_feature_t>::type;