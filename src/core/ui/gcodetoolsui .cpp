#include "gcodetoolsui.h"

void GCodeTools::LoadFileAndSaveExtractedPathAsObject(){
    FilePath file = FileModule::SelectFile();
    if (file.path == nullptr) {
        printf("No file selected.\n");
        return;
    }
    GCodeModule gcode;
    
    gcode.OpenFile(&file);
    gcode.ExtractPointsAndPaths();
    
    printf("GCode Points: %zu, Paths: %zu\n", gcode.points.size(), gcode.paths.size());
    FilePath saveFile = FileModule::SaveFile();
    if (saveFile.path != nullptr) {
        gcode.SavePointsAndPathsToObj(saveFile.path);
    }
}

void GCodeTools::LoadFileIntoProject(Project* project){
    FilePath file = FileModule::SelectFile();
    if (file.path == nullptr) {
        printf("No file selected.\n");
        return;
    }
    printf("Loading GCode file into project: %s\n", file.path);
    project->LoadGCode(&file);
}

void GCodeTools::render() {
    ImGui::Begin("GCode Tools");

    RootUICtx* ctx = GetRootUIContext();
    Project* project = ctx->getProject();

    if(project->isProjectLoaded()){
        FilePath* currentFile = project->GetCurrentGCodeFilePath();
        if(currentFile != nullptr){
            ImGui::Text("Current GCode File: %s", currentFile->path);
            if(ImGui::Button("Genrate Render Object from GCode")){
                project->GenerateRenderObjectFromGCode();
            }
            if(ImGui::Button("Extract Gcode Layers")){
                    project->ExtractLayers();
            }
        } else {
            ImGui::Text("No GCode File Loaded.");
        }
    } else {
        ImGui::Text("No Project Loaded.");
        if(ImGui::Button("Load GCode File into Project")){
            std::thread(LoadFileIntoProject, project).detach();
        }
    }

    if (ImGui::Button("Load GCode File and Extract Paths as OBJ")) {
        std::thread(LoadFileAndSaveExtractedPathAsObject).detach();
    }

    ImGui::End();
}