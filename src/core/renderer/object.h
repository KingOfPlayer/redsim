#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

class Object {
public:
    GLuint VAO;
    GLuint VBO;
    GLuint EBO;
    uint32_t vertexCount;
    GLenum drawMode;
    bool useIndices = true;

    glm::vec4 color; 

    glm::vec3 position = {0.0f, 0.0f, 0.0f};
    glm::vec3 rotation = {0.0f, 0.0f, 0.0f}; 
    glm::vec3 scale    = {1.0f, 1.0f, 1.0f};

    float lineWidth = 1.0f;

    glm::mat4 GetModelMatrix();
};