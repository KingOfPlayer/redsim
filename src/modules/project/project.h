#pragma once
#include <memory>

class GCodeModule;
class LayerMapper;
struct FilePath;
class Object;

class Project {
    std::unique_ptr<GCodeModule> gcodeModule;
    bool isGCodeFileLoaded = false;
    bool isGCodeRenderObjectGenerated = false;
    std::unique_ptr<Object> GCodeRenderObject;

    std::unique_ptr<LayerMapper> layerMapper;
    bool isMeshGenerated = false;
    std::unique_ptr<Object> MeshRenderObject;
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
    void Generate3DMeshFromLayers();
    bool HasMeshGenerated();
    std::unique_ptr<Object>& GetMeshRenderObject();
    LayerMapper& GetLayerMapper();
};