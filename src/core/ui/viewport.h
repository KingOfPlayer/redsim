
#pragma once
#include "imgui.h"
#include "rootui.h"
#include "../renderer/camera.h"
#include "../renderer/renderer.h"
#include "../renderer/shader.h"

class Viewport : public UI {
    Camera* camera;
    Renderer* renderer;
    Object grid;
    GLuint shaderProgram;

public:
    Viewport(RootUICtx* rootUICtx);

    void render() override;
};