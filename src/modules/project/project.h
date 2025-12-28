#include "../gcode/gcode.h"
#include "../modelgen/layermaper.h"

class Project {
GCodeModule gcodeModule;
bool isGCodeFileLoaded = false;
bool isGCodeRenderObjectGenerated = false;
Object GCodeRenderObject;

LayerMapper layerMapper;
bool isMeshGenerated = false;
Object MeshRenderObject;
public:
    Project(){
        gcodeModule = GCodeModule();
        layerMapper = LayerMapper();
    }
    ~Project(){

    }

    void LoadGCode(FilePath* filepath){
        gcodeModule.OpenFile(filepath);
        gcodeModule.ExtractPointsAndPaths();
        isGCodeFileLoaded = true;
    }

    FilePath* GetCurrentGCodeFilePath(){
        return gcodeModule.currentFile;
    }

    void GenerateRenderObjectFromGCode(){
        GCodeRenderObject = gcodeModule.ConvertPathToRenderObject();
        isGCodeRenderObjectGenerated = true;
    }

    bool HasGCodeRenderObject(){
        return isGCodeRenderObjectGenerated;
    }

    Object GetGCodeRenderObject(){
        return GCodeRenderObject;
    }

    bool isProjectLoaded(){
        return isGCodeFileLoaded;
    }

    void ExtractLayers(){
        std::vector<GCodeLayer> layers = gcodeModule.ExtractLayers();

        printf("Extracted %zu layers from GCode.\n", layers.size());
    }

    void Generate3DMeshFromLayers(){
        std::vector<GCodeLayer> layers = gcodeModule.ExtractLayers();
        
        //layerMapper.CreateRenderObjectFromMesh(layers);

        MeshRenderObject = layerMapper.CreateRenderObjectFromMesh(layers);
        isMeshGenerated = true;

        printf("Generated 3D Mesh from Layers.\n");
    }

    bool HasMeshGenerated(){
        return isMeshGenerated;
    }

    Object GetMeshRenderObject(){
        return MeshRenderObject;
    }

    LayerMapper& GetLayerMapper(){
        return layerMapper;
    }
};