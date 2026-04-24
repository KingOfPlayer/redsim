
#pragma once
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

struct GLFWwindow;

class RootUICtx;
class UI;

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