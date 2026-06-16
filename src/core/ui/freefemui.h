#pragma once
#include <imgui.h>

#include "ui.h"

class Project;

class FreeFemUI : public UI {
    double EValue = 3500;
    double PoissonRatioValue = 0.36;

    void render() override;
public:
    FreeFemUI(RootUICtx* rootUI) : UI(rootUI) {}
};