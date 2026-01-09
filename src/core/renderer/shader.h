#pragma once
#include <glad/glad.h>
#include <iostream>
#include <map>

class Shader{
public:
    static GLuint RegisterShaderProgram(const char* vertexSource, const char* fragmentSource);
};


