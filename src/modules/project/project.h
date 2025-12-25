#include "../gcode/gcode.h"
class Project {
GCodeModule gcodeModule;
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
        GenerateRenderObjectFromGCode();
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
};