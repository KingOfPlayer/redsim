#pragma once
#include <memory>
#include <string>

#include "../modelgen/modelgentypes.h"

class GCodeModule;
class LayerMapper;
struct FilePath;

class TetrahedralMesher;
struct TetrahedralMesherResult;

struct VertexGroupBaseType;

class FreeFemScript;
class FreeFemModule;

class FreeFemView;

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
    
    std::unique_ptr<FreeFemView> freefemView;

    bool ViewResultMode = false;
public:
    Project();
    ~Project();
    bool isProjectLoaded();

    std::string GetCurrentFilePath();
    std::string GetFileDirectory();
    std::string GetFilenameWithoutExtension();

    GCodeModule& GetGCodeModuleInstance();
    LayerMapper& GetLayerMapperInstance();
    TetrahedralMesher& GetTetrahedralMesherInstance();
    FreeFemScript& GetFreeFemScriptInstance();
    FreeFemModule& GetFreeFemModuleInstance();
    FreeFemView& GetFreeFemViewInstance();

    void LoadGCode(FilePath* filepath);
    FilePath* GetCurrentGCodeFilePath();
    void GenerateRenderObjectFromGCode();
    bool HasGCodeRenderObject();
    std::unique_ptr<Object>& GetGCodeRenderObject();

    void GenerateShellMesh();
    bool HasShellMeshGenerated();
    std::unique_ptr<Object>& GetMeshRenderObject();

    void GenerateTetrahedralMesh();
    bool HasTetrahedralMeshGenerated();
    std::unique_ptr<Object>& GetTetrahedralMeshMeshRenderObject();
    void ApplyLabel(std::vector<std::unique_ptr<VertexGroupBaseType>> groups);
    void SaveTetrahedralMeshToFile();

    bool LoadSimulationData();
    std::unique_ptr<Object>& GetSimulationRenderObject();

    void ToggleViewResultMode();
    bool isViewResultMode() const;
};