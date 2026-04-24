#include "viewport.h"

#define IMVIEWGUIZMO_IMPLEMENTATION
#include "ImViewGuizmo.h"

#include "../renderer/object.h"
#include "../renderer/camera.h"
#include "../renderer/renderer.h"
#include "../renderer/shader.h"

#include "../../modules/project/project.h"

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

Object CreateGrid(int size, float size_cell, glm::vec4 color) {
    printf("Creating grid of size %d with cell size %.2f\n", size, size_cell);
    Object obj;
    obj.color = color;
    obj.drawMode = GL_LINES;
    obj.useIndices = false;

    std::vector<float> vertices;
    for (int i = -size; i <= size; i++) {
        // Vertical lines (along Z)
        vertices.push_back((float)i * size_cell); vertices.push_back(0); vertices.push_back((float)-size * size_cell);
        vertices.push_back((float)i * size_cell); vertices.push_back(0); vertices.push_back((float)size * size_cell);

        // Horizontal lines (along X)
        vertices.push_back((float)-size * size_cell); vertices.push_back(0); vertices.push_back((float)i * size_cell);
        vertices.push_back((float)size * size_cell);  vertices.push_back(0); vertices.push_back((float)i * size_cell);
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


Viewport::Viewport(RootUICtx* rootUICtx) : UI(rootUICtx) {
    camera = std::make_unique<Camera>(
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        90.0f,
        45.0f,
        4.0f,
        0.5f
    );
    renderer = std::make_unique<Renderer>(800, 600);
    grid = std::make_unique<Object>(CreateGrid(10, 1.0f, glm::vec4(0.7f, 0.7f, 0.7f, 1.0f)));
    shaderProgram = Shader::RegisterShaderProgram(vertexShaderSource, fragmentShaderSource);

    // ImViewGuizmo style configuration
    auto& style = ImViewGuizmo::GetStyle();
    style.scale = 0.75f;
    style.axisLabels[1] = "Z";
    style.axisLabels[2] = "Y";
    style.axisColors[1] = IM_COL32(51, 128, 255, 255);
    style.axisColors[2] = IM_COL32(51, 230, 51, 255);
    style.animateSnap = false;
    style.snapAnimationDuration = 0.75f;
}

void Viewport::render() {
    RootUICtx* ctx = GetRootUIContext();
    Project* project = ctx->getProject();

    ImGui::Begin("Viewport");

    ImVec2 size = ImGui::GetContentRegionAvail();
    ImVec2 pos  = ImGui::GetWindowPos();


    ImVec2 contentMin    = ImGui::GetWindowContentRegionMin();
    ImVec2 contentMax    = ImGui::GetWindowContentRegionMax();
    
    ImVec2 contentStart  = ImVec2(pos.x + contentMin.x, pos.y + contentMin.y);
    ImVec2 contentSize   = ImVec2(contentMax.x - contentMin.x, contentMax.y - contentMin.y);
    
    bool isHovered = ImGui::IsItemHovered(); 
    ImGuiIO& io = ImGui::GetIO();

    // Wiewport rendering
    renderer->SetViewProjection(camera->GetViewMatrix(size.x / size.y));
    renderer->Resize(size.x, size.y);
    renderer->DrawBegin();
    renderer->DrawObject(grid, shaderProgram);

    if(project != nullptr){
        if(project->HasGCodeRenderObject() != false){
            std::unique_ptr<Object>& gcodeObj = project->GetGCodeRenderObject();
            renderer->DrawObject(gcodeObj, shaderProgram);
        }

        if(project->HasMeshGenerated() != false){
            std::unique_ptr<Object>& meshObj = project->GetMeshRenderObject();
            renderer->DrawObject(meshObj, shaderProgram, true);
        }
    }

    renderer->DrawEnd();
    ImGui::Image((void*)(intptr_t)renderer->getFramebufferTexture(), size,
        ImVec2(1, 0),  // top-left corner of the texture
        ImVec2(0, 1)   // bottom-right corner of the texture
    );

    // --- CAMERA MOVEMENT SECTION ---
    bool cameraUpdated = false;
    if (ImGui::IsWindowHovered() && !ImViewGuizmo::IsOver()) {
        // Pan
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle)) {
            camera->Pan(-io.MouseDelta.x, io.MouseDelta.y);
            cameraUpdated = true;
        } else
        // Orbit
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Right)) {
            camera->Orbit(io.MouseDelta.x, io.MouseDelta.y);
            cameraUpdated = true;
        }
        if(ImGui::GetIO().MouseWheel != 0.0f) {
            camera->Zoom(io.MouseWheel);
            cameraUpdated = true;
        }
    }

    // Debug
    /*if (cameraUpdated) {
        glm::vec3 cameraPos = camera->GetPosition();
        glm::vec3 cameraTarget = camera->GetTarget();
        printf("Camara Position: (%.2f, %.2f, %.2f)\n", cameraPos.x, cameraPos.y, cameraPos.z);
        printf("Camara Target Position: (%.2f, %.2f, %.2f)\n", cameraTarget.x, cameraTarget.y, cameraTarget.z);
    }*/

    // ImViewGuizmo
    float padding   = 30.f;
    float gizmoSize = 128.f / 2 * 0.75f;

    ImVec2 gizmoPos = ImVec2(
        contentStart.x + padding + gizmoSize,
        contentStart.y + contentSize.y - gizmoSize - padding
    );

    glm::vec3 cameraPos = camera->GetPosition();
    glm::quat cameraRot = camera->GetRotation();
    glm::vec3 cameraTarget = camera->GetTarget();
    ImViewGuizmo::BeginFrame();

    ImViewGuizmo::Rotate(cameraPos, cameraRot,cameraTarget, gizmoPos);

    ImGui::End();

}