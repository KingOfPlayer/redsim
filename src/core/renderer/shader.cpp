#include "shader.h"
#include <fstream>
#include <sstream>
#include <cassert>

// Initialize the static cache
std::map<std::string, GLuint> ShaderFactory::m_ShaderCache;

GLuint ShaderFactory::RegisterFromSource(const std::string& name, const char* vertexSource, const char* fragmentSource) {
    // 1. Check if it already exists to avoid redundant compilation
    if (m_ShaderCache.find(name) != m_ShaderCache.end()) {
        std::cout << "Warning: Shader program '" << name << "' already exists. Returning cached ID.\n";
        return m_ShaderCache[name];
    }

    // 2. Compile Vertex Shader
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertexSource, NULL);
    glCompileShader(vs);
    CheckCompileErrors(vs, "VERTEX");

    // 3. Compile Fragment Shader
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragmentSource, NULL);
    glCompileShader(fs);
    CheckCompileErrors(fs, "FRAGMENT");

    // 4. Link Program
    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    CheckCompileErrors(program, "PROGRAM");

    // 5. Clean up intermediate shaders
    glDeleteShader(vs);
    glDeleteShader(fs);

    // 6. Cache and return
    m_ShaderCache[name] = program;
    std::cout << "Shader '" << name << "' registered successfully. ID: " << program << std::endl;
    
    return program;
}

GLuint ShaderFactory::GetProgram(const std::string& name) {
    auto it = m_ShaderCache.find(name);
    if (it != m_ShaderCache.end()) {
        return it->second;
    }
    std::cerr << "ERROR: Shader '" << name << "' not found in cache!\n";
    return 0;
}

void ShaderFactory::ClearCache() {
    for (auto const& [name, id] : m_ShaderCache) {
        glDeleteProgram(id);
    }
    m_ShaderCache.clear();
    std::cout << "Shader cache cleared.\n";
}

bool ShaderFactory::CheckCompileErrors(GLuint shader, std::string type) {
    GLint success;
    GLchar infoLog[1024];
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "ERROR: Shader Compilation Failed of type: " << type << "\n" << infoLog << "\n-------------------------------------------------\n";
            return false;
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "ERROR: Program Linking Failed of type: " << type << "\n" << infoLog << "\n-------------------------------------------------\n";
            return false;
        }
    }
    return true;
}
