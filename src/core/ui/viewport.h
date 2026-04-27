
#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <array>

#include "imgui.h"
#include "ui.h"

class Camera;
class Object;
class Renderer;

class Viewport : public UI {
    std::unique_ptr<Camera> camera;
    std::unique_ptr<Renderer> renderer;
    std::unique_ptr<Object> grid;
    GLuint shaderProgram;

public:
    Viewport(RootUICtx* rootUICtx);

    void render() override;
};