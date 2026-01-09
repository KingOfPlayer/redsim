#pragma once

#include <imgui.h>
#include "rootui.h"
#include "../../modules/file/file.h"
#include "../../modules/gcode/gcode.h"

class GCodeTools : public UI {
    static void LoadFileAndSaveExtractedPathAsObject();
    static void LoadFileIntoProject(Project* project);

    void render() override;
public:
    GCodeTools(RootUICtx* rootUI) : UI(rootUI) {}
};