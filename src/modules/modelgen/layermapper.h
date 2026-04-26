#pragma once

#include <deque>
#include <future>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "../../core/renderer/object.h"

#include "../gcode/gcode.h"

#include <execution>
#include <algorithm>
#include <numeric>
#include <vector>
#include <mutex>

#include "modelgen.h"

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
    float remesh_target_length = 0.1f;

    // Merge
    static Mesh MergeLayersToModel(std::vector<Mesh> layers);
    static Nef_polyhedron MeshToNef(const Mesh& m);
    static Mesh NefToMesh(const Nef_polyhedron& nef);
    static Mesh MergeLayersToModelWithNef(std::vector<Mesh> layers);
    static Mesh MergeTwoMesh(Mesh m1, Mesh m2);
    static void ShiftLayerMesh(Mesh& extruded_layer, float layer_offset, float layer_height);

    LayerMapper();

    void Set2DNozzlePolygon(float diameter);
    Polygon_2 place_nozzle_at(Polygon_2 nozzle, Point_2 vertex);

    std::vector<Polygon_with_holes_2> GCodePathsToPolygons(std::vector<GCodePoint> points, std::vector<GCodePath> paths);
    Mesh PolygonsLayerToMesh(std::vector<Polygon_with_holes_2>& layer, float layer_height);

    Mesh RemeshModel(Mesh model);

    Mesh GenerateMesh(std::vector<GCodeLayer> layers);
    Object MeshToRenderObject(const Mesh mesh);
};
