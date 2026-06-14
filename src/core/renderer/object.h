#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <string>
#include <vector>
#include <variant>

using UniformValue = std::variant<int, float, glm::vec3, glm::vec4, glm::mat4>;

struct Uniform {
    std::string name;
    UniformValue value;
};

struct UniformApplier {
    unsigned int shaderProgram;
    const std::string& name;

    void operator()(int v) const {
        glUniform1i(glGetUniformLocation(shaderProgram, name.c_str()), v);
    }
    void operator()(float v) const {
        glUniform1f(glGetUniformLocation(shaderProgram, name.c_str()), v);
    }
    void operator()(const glm::vec3& v) const {
        glUniform3fv(glGetUniformLocation(shaderProgram, name.c_str()), 1, &v[0]);
    }
    void operator()(const glm::vec4& v) const {
        glUniform4fv(glGetUniformLocation(shaderProgram, name.c_str()), 1, &v[0]);
    }
    void operator()(const glm::mat4& v) const {
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, name.c_str()), 1, GL_FALSE, &v[0][0]);
    }
};

class Object {
public:
    GLuint VAO;
    GLuint VBO;
    GLuint EBO;
    uint32_t vertexCount;
    GLenum drawMode;
    bool useIndices = true;
    std::vector<Uniform> uniforms;

    glm::vec3 position = {0.0f, 0.0f, 0.0f};
    glm::vec3 rotation = {0.0f, 0.0f, 0.0f}; 
    glm::vec3 scale    = {1.0f, 1.0f, 1.0f};

    std::vector<float> vertices;
    std::vector<uint32_t> indices;

    float lineWidth = 1.0f;

    glm::mat4 GetModelMatrix();

    void setUniform(const std::string& name, const UniformValue& value) {
        for (auto& u : uniforms) {
            if (u.name == name) {
                u.value = value;
                return;
            }
        }
        uniforms.push_back({name, value});
    }

    const std::vector<Uniform>& getUniforms() const { return uniforms; }

    const UniformValue* getUniformValue(const std::string& name) const {
        for (const auto& u : uniforms) {
            if (u.name == name) {
                return &u.value;
            }
        }
        return nullptr;
    }
};