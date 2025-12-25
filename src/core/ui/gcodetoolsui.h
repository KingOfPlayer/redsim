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
    void render() override {
        ImGui::Begin("GCode Tools");

        if (ImGui::Button("Click me")) {
            printf("Button clicked!\n");
            std::thread(LoadFileAndSaveExtractedPathAsObject).detach();
        }

        ImGui::End();
    }


public:
    GCodeTools(RootUI* rootUI) : UI(rootUI) {}
};