#include "imgui.h"
#include "rootui.h"
#include "../renderer/renderer.h"
#include "../renderer/shader.h"
#pragma once

const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    uniform mat4 u_CombinedMatrix;
    void main() {
        gl_Position = u_CombinedMatrix * vec4(aPos, 1.0);
    }
)";

const char* fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;
    uniform vec4 u_Color;
    void main() {
        FragColor = u_Color;
    }
)";

Object CreatePlane(float size, glm::vec4 color) {
    Object obj;
    obj.color = color;
    obj.drawMode = GL_TRIANGLES;
    
    float halfSize = size / 2.0f;

    // 1. Define Vertex Positions (X, Y, Z)
    float vertices[] = {
        -halfSize, 0.0f,  halfSize, // 0: Bottom-Left
         halfSize, 0.0f,  halfSize, // 1: Bottom-Right
         halfSize, 0.0f, -halfSize, // 2: Top-Right
        -halfSize, 0.0f, -halfSize  // 3: Top-Left
    };

    // 2. Define Indices (Two triangles)
    unsigned int indices[] = {
        0, 1, 2, // First Triangle
        2, 3, 0  // Second Triangle
    };
    obj.vertexCount = 6; // Total indices to draw

    // 3. Generate and Bind VAO
    glGenVertexArrays(1, &obj.VAO);
    glBindVertexArray(obj.VAO);

    // 4. Create VBO (Vertex Data)
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 5. Create EBO (Index Data)
    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // 6. Set Attribute Protocol (Location 0 = Position)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Unbind to stay clean
    glBindVertexArray(0);

    return obj;
}

Object CreateGrid(int size, float step, glm::vec4 color) {
    Object obj;
    obj.color = color;
    obj.drawMode = GL_LINES;
    obj.useIndices = false;

    std::vector<float> vertices;
    for (int i = -size; i <= size; i++) {
        // Vertical lines (along Z)
        vertices.push_back((float)i * step); vertices.push_back(0); vertices.push_back((float)-size * step);
        vertices.push_back((float)i * step); vertices.push_back(0); vertices.push_back((float)size * step);

        // Horizontal lines (along X)
        vertices.push_back((float)-size * step); vertices.push_back(0); vertices.push_back((float)i * step);
        vertices.push_back((float)size * step);  vertices.push_back(0); vertices.push_back((float)i * step);
    }

    obj.vertexCount = (uint32_t)vertices.size() / 3;

    glGenVertexArrays(1, &obj.VAO);
    glBindVertexArray(obj.VAO);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    return obj;
}

class Viewport : public UI {
    Renderer* renderer;
    Object plane;
    Object grid;
    GLuint shaderProgram;

    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    float yaw = -90.0f; 
    float pitch = -20.0f;
    float distance = 5.0f;
    float sensitivity = 0.5f;

public:
    Viewport(RootUICtx* rootUICtx) : UI(rootUICtx) {
        renderer = new Renderer(800, 600);
        plane = CreatePlane(1.0f, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
        grid = CreateGrid(10, 1.0f, glm::vec4(0.7f, 0.7f, 0.7f, 1.0f));
        shaderProgram = Shader::RegisterShaderProgram(vertexShaderSource, fragmentShaderSource);
    }

    void render() override {
        RootUICtx* ctx = GetRootUIContext();
        Project* project = ctx->getProject();

        ImGui::Begin("Viewport");

        ImVec2 size = ImGui::GetContentRegionAvail();
        
        bool isHovered = ImGui::IsItemHovered(); 
        ImGuiIO& io = ImGui::GetIO();

        // --- CAMERA MOVEMENT SECTION ---
        bool cameraUpdated = false;
        if (ImGui::IsWindowHovered()) {
            // Pan
            if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle)) {
                ImVec2 delta = io.MouseDelta;

                // Calculate camera vectors
                glm::vec3 forward = glm::normalize(cameraTarget - cameraPos);
                glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));
                glm::vec3 up = glm::normalize(glm::cross(right, forward));

                float panSpeed = distance * sensitivity / 100.0f; 
                glm::vec3 offset = (right * delta.x * panSpeed) + (up * -delta.y * panSpeed);
                
                cameraTarget += offset;
                cameraPos += offset;
                cameraUpdated = true;
            } else
            // Orbit
            if (ImGui::IsMouseDragging(ImGuiMouseButton_Right)) {
                ImVec2 delta = ImGui::GetIO().MouseDelta;
                yaw   += delta.x * sensitivity; // Sensitivity
                pitch += delta.y * sensitivity;

                
                if (pitch > 89.0f) pitch = 89.0f;
                if (pitch < -89.0f) pitch = -89.0f;
                cameraUpdated = true;
            }
            if(ImGui::GetIO().MouseWheel != 0.0f) {
                distance -= ImGui::GetIO().MouseWheel * 0.5f;
                if (distance < 1.0f) distance = 1.0f;
                cameraUpdated = true;
            }
        }

        // Calculate New Camera Position based on Yaw/Pitch (Orbit Math)
        cameraPos.x = cameraTarget.x + cos(glm::radians(yaw)) * cos(glm::radians(pitch)) * distance;
        cameraPos.y = cameraTarget.y + sin(glm::radians(pitch)) * distance;
        cameraPos.z = cameraTarget.z + sin(glm::radians(yaw)) * cos(glm::radians(pitch)) * distance;

        // --- MATRIX CALCULATION ---
        glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, glm::vec3(0,1,0));
        float aspect = size.x / size.y;
        glm::mat4 proj = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
        glm::mat4 viewProj = proj * view;
        //Debugging: Print the view-projection matrix
        if (cameraUpdated) {
            printf("Camara Position: (%.2f, %.2f, %.2f)\n", cameraPos.x, cameraPos.y, cameraPos.z);
            printf("Camara Target Position: (%.2f, %.2f, %.2f)\n", cameraTarget.x, cameraTarget.y, cameraTarget.z);
        }

        renderer->SetViewProjection(viewProj);

        if (size.x != renderer->Viewport_Width || size.y != renderer->Viewport_Height) {
            renderer->Viewport_Width = size.x;
            renderer->Viewport_Height = size.y;

            renderer->Resize(renderer->Viewport_Width, renderer->Viewport_Height);
        }

        renderer->DrawBegin();
        
        // Test Objects
        //renderer->DrawObject(plane, shaderProgram);

        renderer->DrawObject(grid, shaderProgram);

        if(project != nullptr){
            if(project->HasGCodeRenderObject() != false){
                Object gcodeObj = project->GetGCodeRenderObject();
                renderer->DrawObject(gcodeObj, shaderProgram);
            }
        }

        renderer->DrawEnd();
        ImGui::Image((void*)(intptr_t)renderer->FBOTexture, size,
            ImVec2(1, 0),  // top-left corner of the texture
            ImVec2(0, 1)   // bottom-right corner of the texture
        );
        ImGui::End();
    }
};