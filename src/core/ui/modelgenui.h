
#pragma once
#include <vector>
#include <imgui.h>

#include "ui.h"


class ModelGenUI : public UI{
    float nozzleDiameter = 0.46f;
    int qualityIndex = 0;
    bool nef_based = false;
    bool remesh_after_layers = false;
    void render() override;
public:
    ModelGenUI(RootUICtx* rootUICtx);
};