
#pragma once
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "rootuictx.h"
#include "ui.h"
#include "gcodetoolsui.h"
#include "viewport.h"
#include "modelgenui.h"

class RootUI {
RootUICtx* rootUICtx;
ImGuiIO* io;
std::vector<UI*> uiComponents;
public:
    RootUI(GLFWwindow* window, RootUICtx* ctx);

    ~RootUI();
    void addUI(UI* component);
    void render();

    RootUICtx* getCtx();
};