#include <imgui.h>
#include "rootui.h"
#include "../../modules/file/file.h"
#include "../../modules/gcode/gcode.h"
#pragma once

class GCodeTools : public UI {
    static void LoadFileAndSaveExtractedPathAsObject(){
        FilePath file = FileModule::SelectFile();
        if (file.path == nullptr) {
            printf("No file selected.\n");
            return;
        }
        GCodeModule gcode;
        
        gcode.OpenFile(&file);
        gcode.ExtractPointsAndPaths();

        // For demonstration, print the number of points and paths extracted
        printf("GCode Points: %zu, Paths: %zu\n", gcode.points.size(), gcode.paths.size());
        FilePath saveFile = FileModule::SaveFile();
        if (saveFile.path != nullptr) {
            gcode.SavePointsAndPathsToObj(saveFile.path);
        }
    }

    static void LoadFileIntoProject(Project* project){
        FilePath file = FileModule::SelectFile();
        if (file.path == nullptr) {
            printf("No file selected.\n");
            return;
        }
        printf("Loading GCode file into project: %s\n", file.path);
        project->LoadGCode(&file);
    }

    void render() override {
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


public:
    GCodeTools(RootUICtx* rootUI) : UI(rootUI) {}
};