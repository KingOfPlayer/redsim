#include "shader.h"

GLuint Shader::RegisterShaderProgram(const char* vertexSource, const char* fragmentSource) {
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertexSource, NULL);
    glCompileShader(vs);

    int success;
    char infoLog[512];
    glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vs, 512, NULL, infoLog);
        std::cout << "ERROR: Vertex Shader Compilation Failed\n" << infoLog << std::endl;
    }

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragmentSource, NULL);
    glCompileShader(fs);

    glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fs, 512, NULL, infoLog);
        std::cout << "ERROR: Fragment Shader Compilation Failed\n" << infoLog << std::endl;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cout << "ERROR: Shader Program Linking Failed\n" << infoLog << std::endl;
    }


    glDeleteShader(vs);
    glDeleteShader(fs);
    while (GLenum error = glGetError()) {
        std::cout << "OpenGL Error during shader compilation/linking: " << error << std::endl;
    }

    printf("Shader program compiled and linked successfully. Program ID: %u\n", program);
    

    return program;
}


