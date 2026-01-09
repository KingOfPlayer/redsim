#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <thread>
#include "ui/rootui.h"
#include "ui/gcodetoolsui.h"
#include "ui/viewport.h"
#include "ui/rootuictx.h"

class Window{
GLFWwindow* window;
Project* project;
RootUICtx* rootUICtx;
RootUI* rootUI;
public:
    Window(int width, int height, const char* title);

    void update();

    ~Window();

    bool isWindowShouldClose();
};