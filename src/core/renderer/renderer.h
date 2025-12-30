#include <glad/glad.h>
#include <cstdio>
#include "object.h"
#pragma once

class Renderer {
public:
    glm::mat4 viewProj;
    glm::vec3 globalScale = glm::vec3(0.1f, 0.1f, 0.1f);
    int Viewport_Width, Viewport_Height;

    GLuint FBO;
    GLuint RBO;
    GLuint FBOTexture;
    
    Renderer(int width, int height): Viewport_Width(width), Viewport_Height(height) {
        // Generate FBO
        glGenFramebuffers(1, &FBO);
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);

        // Create texture to render to
        glGenTextures(1, &FBOTexture);
        glBindTexture(GL_TEXTURE_2D, FBOTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Attach texture to FBO
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, FBOTexture, 0);

        glGenRenderbuffers(1, &RBO);
        glBindRenderbuffer(GL_RENDERBUFFER, RBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

        // Check if FBO is complete
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            printf("ERROR: Framebuffer is not complete!\n");
        }

        // Unbind FBO
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        viewProj = glm::lookAt(
            glm::vec3(4.0f, 3.0f, 3.0f), 
            glm::vec3(0.0f, 0.0f, 0.0f), 
            glm::vec3(0.0f, 1.0f, 0.0f)
        );
    }

    void Resize(int width, int height) {
        glBindTexture(GL_TEXTURE_2D, FBOTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glBindTexture(GL_TEXTURE_2D, 0);

        glBindRenderbuffer(GL_RENDERBUFFER, RBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }

    void DrawBegin() {
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        glViewport(0, 0, Viewport_Width, Viewport_Height); // Ensure viewport matches FBO size
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f); // Professional dark grey background
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    void SetViewProjection(const glm::mat4& vp) {
        viewProj = vp;
    }

    void DrawObject(class Object& obj, GLuint shaderProgram, bool wireframe=false) {
        glUseProgram(shaderProgram);

        // Calculate MVP: Perspective * View * Model
        glm::mat4 model = obj.GetModelMatrix();
        model = glm::scale(model, globalScale);
        glm::mat4 mvp = viewProj * model; 

        if(obj.drawMode == GL_LINES) {
            glLineWidth(obj.lineWidth);
        }

        // Send to the uniform you defined in your vertex shader
        GLint mvpLoc = glGetUniformLocation(shaderProgram, "u_CombinedMatrix");
        glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));

        // Send Color
        GLint colorLoc = glGetUniformLocation(shaderProgram, "u_Color");
        glUniform4fv(colorLoc, 1, glm::value_ptr(obj.color));

        glBindVertexArray(obj.VAO);

        if(obj.useIndices)
            glDrawElements(obj.drawMode, obj.vertexCount, GL_UNSIGNED_INT, 0);
        else
            glDrawArrays(obj.drawMode, 0, obj.vertexCount);

        if(wireframe && obj.drawMode == GL_TRIANGLES) {
            glDepthMask(GL_FALSE);
            glEnable(GL_POLYGON_OFFSET_LINE);
            glPolygonOffset(-1.0f, -1.0f); // Pull wireframe closer to camera

            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

            glm::vec4 wireColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f); 
            glUniform4fv(colorLoc, 1, glm::value_ptr(wireColor));

            if(obj.useIndices)
                glDrawElements(obj.drawMode, obj.vertexCount, GL_UNSIGNED_INT, 0);
            else
                glDrawArrays(obj.drawMode, 0, obj.vertexCount);
                
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            glDisable(GL_POLYGON_OFFSET_LINE);
            glDepthMask(GL_TRUE);
        }

        glBindVertexArray(0);
    }

    void DrawEnd() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    ~Renderer() {
        glDeleteFramebuffers(1, &FBO);
        glDeleteTextures(1, &FBOTexture);
    }
};