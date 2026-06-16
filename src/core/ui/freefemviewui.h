#pragma once
#include <imgui.h>

#include "ui.h"

#include "../../modules/freefem/freefemview.h"

class Project;
class RootUICtx;

class FreeFemViewUI : public UI {
    SimulationViewSettings viewSettings;

    double minScalar = 0.0;
    double maxScalar = 1.0;

    void render() override;
public:
    FreeFemViewUI(RootUICtx* rootUI);
};