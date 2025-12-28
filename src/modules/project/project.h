#include "../gcode/gcode.h"
class Project {
GCodeModule gcodeModule;
bool isGCodeFileLoaded = false;
bool isGCodeRenderObjectGenerated = false;
Object GCodeRenderObject;
public:
    Project(){
        gcodeModule = GCodeModule();
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
};