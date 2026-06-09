
#pragma once
#include <vector>
#include <imgui.h>

#include "ui.h"

class LabelUI : public UI{

    void render() override;
public:
    LabelUI(RootUICtx* rootUICtx);
};