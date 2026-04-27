#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "modelgentypes.h"
#include "layermapper.h"
#include "tetrahedralmesher.h"

class Object;

class ModelgenHelper {
public:
    static Object MeshToRenderObject(const Mesh mesh);
    static Mesh_fast MeshToMeshFast(const Mesh mesh);
    static Mesh MeshFastToMesh(const Mesh_fast mesh);
};