#pragma once
#include <glad/glad.h>
#include <thread>
#include <cstdio>
#include <cstring>

class Project;
class RootUICtx;
class RootUI;
struct GLFWwindow;

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