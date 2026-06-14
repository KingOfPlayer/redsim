#pragma once
#include <glad/glad.h>
#include <iostream>
#include <map>

class ShaderFactory{
public:
    ShaderFactory() = delete;

    static GLuint RegisterFromSource(const std::string& name, const char* vertexSource, const char* fragmentSource);
    
    static GLuint GetProgram(const std::string& name);

    static void ClearCache();
private:
    static std::map<std::string, GLuint> m_ShaderCache;
    static bool CheckCompileErrors(GLuint shader, std::string type);
};


