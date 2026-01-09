

#pragma once
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
#include <CGAL/Polygon_mesh_processing/detect_features.h>

#include <CGAL/Polyhedron_3.h>
#include <CGAL/Nef_polyhedron_3.h>
#include <CGAL/boost/graph/convert_nef_polyhedron_to_polygon_mesh.h>

#include <boost/property_map/property_map.hpp>
#include <deque>
#include <future>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "../gcode/gcode.h"

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

struct Nozzle2D {
    Polygon_2 polygon;
    float diameter;
};

class LayerMapper
{
public:
    enum NozzleQuality
    {
        LOW = 0,
        MEDIUM = 1,
        HIGH = 2
    }; 
    NozzleQuality nozzleQuality = MEDIUM;
    Nozzle2D nozzle;
    bool Nef_based = false;
    bool remesh_after_layers = false;

    LayerMapper();

    void Set2DNozzlePolygon(float diameter);
    Polygon_2 place_nozzle_at(Polygon_2 nozzle, Point_2 vertex);
    std::list<Polygon_with_holes_2> GenerateLayerFromPaths(std::vector<GCodePoint> points, std::vector<GCodePath> paths);
    Mesh Extrude2DLayerTo3D(std::list<Polygon_with_holes_2> layer, float layer_height);
    static Mesh MergeTwoMesh(Mesh m1, Mesh m2);
    static Mesh UnionMergeLayersToModel(std::vector<Mesh> layers);
    Mesh RemeshModel(Mesh model, float target_edge_length);
    Object MeshToRenderObject(const Mesh mesh);
    static Nef_polyhedron MeshToNef(const Mesh& m);
    static Mesh NefToMesh(const Nef_polyhedron& nef);
    static Mesh NefMergeLayersToModel(std::vector<Mesh> layers);
    Object CreateRenderObjectFromMesh(std::vector<GCodeLayer> layers);
    void ShiftLayer(Mesh& extruded_layer, float layer_offset, float layer_height);
};
