#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <cstdio>
#include "object.h"

class Renderer {
    glm::mat4 viewProj;
    glm::vec3 globalScale = glm::vec3(0.1f, 0.1f, 0.1f);
    int Viewport_Width, Viewport_Height;

    GLuint FBO;
    GLuint RBO;
    GLuint FBOTexture;
public:
    
    Renderer(int width, int height);

    void Resize(int width, int height);

    void CreateFramebuffer(int width, int height);

    void DrawBegin();

    void SetViewProjection(const glm::mat4& vp);

    void DrawObject(class Object& obj, GLuint shaderProgram, bool wireframe=false, bool depthMask=true);

    void DrawEnd();

    GLuint getFramebufferTexture();

    ~Renderer();
};