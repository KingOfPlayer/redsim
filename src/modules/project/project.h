#pragma once
#include <memory>
#include <string>

#include "../modelgen/modelgentypes.h"

class GCodeModule;
class LayerMapper;
struct FilePath;

class TetrahedralMesher;
struct TetrahedralMesherResult;

class FreeFemScript;
class FreeFemModule;

struct VertexGroupBaseType;

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
    std::unique_ptr<Object> TetrahedralMeshRenderObject;
    bool isTetrahedralMeshSaved = false;
    
    std::unique_ptr<FreeFemScript> freefemScript;
    std::unique_ptr<FreeFemModule> freefemModule;
public:
    Project();
    ~Project();
    bool isProjectLoaded();

    std::string GetCurrentFilePath();
    std::string GetFileDirectory();
    std::string GetFilenameWithoutExtension();

    void LoadGCode(FilePath* filepath);
    FilePath* GetCurrentGCodeFilePath();
    void GenerateRenderObjectFromGCode();
    bool HasGCodeRenderObject();
    std::unique_ptr<Object>& GetGCodeRenderObject();

    void ExtractLayers();
    void GenerateShellMesh();
    bool HasShellMeshGenerated();
    std::unique_ptr<Object>& GetMeshRenderObject();
    LayerMapper& GetLayerMapper();

    void GenerateTetrahedralMesh();
    bool HasTetrahedralMeshGenerated();
    std::unique_ptr<Object>& GetTetrahedralMeshMeshRenderObject();
    void ApplyLabel(std::vector<std::unique_ptr<VertexGroupBaseType>> groups);
    void SaveTetrahedralMeshToFile();
    TetrahedralMesher& GetTetrahedralMesher();

    FreeFemScript& GetFreeFemScriptInstance();
    FreeFemModule& GetFreeFemModuleInstance();

};