#pragma once
#include <imgui.h>

#include "ui.h"

class Project;

class GCodeTools : public UI {
    static void LoadFileAndSaveExtractedPathAsObject();
    static void LoadFileIntoProject(Project* project);

    void render() override;
public:
    GCodeTools(RootUICtx* rootUI) : UI(rootUI) {}
};