#pragma once
#include <memory>
#include <string>

#include "../modelgen/modelgentypes.h"

class GCodeModule;
class LayerMapper;
struct FilePath;

class TetrahedralMesher;
struct TetrahedralMesherResult;

class Object;

class Project {
    std::unique_ptr<GCodeModule> gcodeModule;
    bool isGCodeFileLoaded = false;
    bool isGCodeRenderObjectGenerated = false;
    std::unique_ptr<Object> GCodeRenderObject;

    std::unique_ptr<LayerMapper> layerMapper;
    bool isMeshGenerated = false;
    std::unique_ptr<Mesh> shellMesh;
    std::unique_ptr<Object> MeshRenderObject;

    std::unique_ptr<TetrahedralMesher> tetrahedralMesher;
    bool isTetrahedralMeshGenerated = false;
    std::unique_ptr<TetrahedralMesherResult> tetrahedralMeshResult;

public:
    Project();
    ~Project();
    bool isProjectLoaded();

    void LoadGCode(FilePath* filepath);
    FilePath* GetCurrentGCodeFilePath();
    void GenerateRenderObjectFromGCode();
    bool HasGCodeRenderObject();
    std::unique_ptr<Object>& GetGCodeRenderObject();

    void ExtractLayers();
    void GenerateShellMesh();
    bool HasMeshGenerated();
    std::unique_ptr<Object>& GetMeshRenderObject();
    LayerMapper& GetLayerMapper();

    void GenerateTetrahedralMesh();
    bool HasTetrahedralMeshGenerated();
    void SaveTetrahedralMeshToFile();

};