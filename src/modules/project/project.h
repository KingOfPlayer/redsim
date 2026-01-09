#include "../gcode/gcode.h"
#include "../modelgen/layermapper.h"

class Project {
    GCodeModule gcodeModule;
    bool isGCodeFileLoaded = false;
    bool isGCodeRenderObjectGenerated = false;
    Object GCodeRenderObject;

    LayerMapper layerMapper;
    bool isMeshGenerated = false;
    Object MeshRenderObject;
public:
    Project();
    ~Project();
    bool isProjectLoaded();

    void LoadGCode(FilePath* filepath);
    FilePath* GetCurrentGCodeFilePath();
    void GenerateRenderObjectFromGCode();
    bool HasGCodeRenderObject();
    Object GetGCodeRenderObject();

    void ExtractLayers();
    void Generate3DMeshFromLayers();
    bool HasMeshGenerated();
    Object GetMeshRenderObject();
    LayerMapper& GetLayerMapper();
};