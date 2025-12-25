
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#pragma once

class Object {
public:
    GLuint VAO;
    uint32_t vertexCount;
    GLenum drawMode;
    bool useIndices = true;

    glm::vec4 color; 

    glm::vec3 position = {0.0f, 0.0f, 0.0f};
    glm::vec3 rotation = {0.0f, 0.0f, 0.0f}; 
    glm::vec3 scale    = {1.0f, 1.0f, 1.0f};

    float lineWidth = 1.0f;

    // Helper to generate the Model Matrix
    glm::mat4 GetModelMatrix() {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position);
        // Apply rotations
        model = glm::rotate(model, glm::radians(rotation.x), {1, 0, 0});
        model = glm::rotate(model, glm::radians(rotation.y), {0, 1, 0});
        model = glm::rotate(model, glm::radians(rotation.z), {0, 0, 1});
        model = glm::scale(model, scale);
        return model;
    }
};