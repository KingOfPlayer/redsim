#include <glad/glad.h>
#include <iostream>
#include <map>

class Shader{
public:
    
    static GLuint RegisterShaderProgram(const char* vertexSource, const char* fragmentSource) {
        // 1. Create and Compile Vertex Shader
        GLuint vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs, 1, &vertexSource, NULL);
        glCompileShader(vs);

        // Check for Vertex errors
        int success;
        char infoLog[512];
        glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(vs, 512, NULL, infoLog);
            std::cout << "ERROR: Vertex Shader Compilation Failed\n" << infoLog << std::endl;
        }

        // 2. Create and Compile Fragment Shader
        GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs, 1, &fragmentSource, NULL);
        glCompileShader(fs);

        // Check for Fragment errors
        glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(fs, 512, NULL, infoLog);
            std::cout << "ERROR: Fragment Shader Compilation Failed\n" << infoLog << std::endl;
        }

        // 3. Link Shaders into a Program
        GLuint program = glCreateProgram();
        glAttachShader(program, vs);
        glAttachShader(program, fs);
        glLinkProgram(program);

        // Check for Linking errors
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(program, 512, NULL, infoLog);
            std::cout << "ERROR: Shader Program Linking Failed\n" << infoLog << std::endl;
        }

        // 4. Cleanup (Shaders are linked, we don't need the individual stages anymore)
        glDeleteShader(vs);
        glDeleteShader(fs);
        
        return program;
    }
};


