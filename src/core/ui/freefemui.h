#pragma once
#include <imgui.h>

#include "ui.h"

#include "../../modules/project/project.h"

class Project;

class FreefemUI : public UI {
    double EValue = 3500;
    double PoissonRatioValue = 0.36;

    void render() override;
public:
    FreefemUI(RootUICtx* rootUI) : UI(rootUI) {}
};