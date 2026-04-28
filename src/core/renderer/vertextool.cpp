
#include "renderconst.h"
#include "vertextool.h"
#include <cstdio>

std::vector<glm::vec3> VertexTool::selectedVertices;

glm::vec3 VertexTool::GetWorldRayFromScreen(float x, float y, const glm::mat4& projection, const glm::mat4& view)
{
    float ndcX = (2.0f * x) - 1.0f;
    float ndcY = 1.0f - (2.0f * y);

    glm::vec4 clipCoords(ndcX, ndcY, -1.0f, 1.0f);

    glm::vec4 eyeCoords = glm::inverse(projection) * clipCoords;
    eyeCoords.z = -1.0f;
    eyeCoords.w = 0.0f;

    glm::vec4 worldRay = glm::inverse(view) * eyeCoords;
    return glm::normalize(glm::vec3(worldRay));
}

std::vector<glm::vec3> VertexTool::SelectVertices(const std::unique_ptr<Object>& object, glm::vec2 start, glm::vec2 end, const glm::mat4& proj, const glm::mat4& view, const glm::vec3& cameraPos)
{
    selectedVertices.clear();
    
    // Frustum ray edge
    glm::vec3 rayTopLeft     = VertexTool::GetWorldRayFromScreen(start.x, start.y, proj, view);
    glm::vec3 rayBottomLeft  = VertexTool::GetWorldRayFromScreen(start.x, end.y,   proj, view);
    glm::vec3 rayTopRight    = VertexTool::GetWorldRayFromScreen(end.x,   start.y, proj, view);
    glm::vec3 rayBottomRight = VertexTool::GetWorldRayFromScreen(end.x,   end.y,   proj, view);

    std::vector<VertexTool::FrustumPlane> frustumPlanes;
    frustumPlanes.emplace_back(glm::cross(rayBottomLeft, rayTopLeft), cameraPos);     // Left plane
    frustumPlanes.emplace_back(glm::cross(rayBottomRight, rayBottomLeft), cameraPos);  // Bottom plane
    frustumPlanes.emplace_back(glm::cross(rayTopRight, rayBottomRight), cameraPos);    // Right plane
    frustumPlanes.emplace_back(glm::cross(rayTopLeft, rayTopRight), cameraPos);        // Top plane

    glm::mat4 modelMatrix = object->GetModelMatrix();
    glm::vec3 globalScale = glm::vec3(RENDERSCALE, RENDERSCALE, RENDERSCALE);
    modelMatrix = glm::scale(modelMatrix, globalScale);

    // Check each vertex against the frustum planes
    for (size_t i = 0; i < object->vertices.size(); i += 3) {
        glm::vec3 vertexPos(object->vertices[i], object->vertices[i + 1], object->vertices[i + 2]);

        // Transform vertex to world space (with same transforms as rendering)
        vertexPos = glm::vec3(modelMatrix * glm::vec4(vertexPos, 1.0f));

        bool isSelected = true;
        for (const auto& plane : frustumPlanes) {
            if (!plane.isPointInFront(vertexPos)) {
                isSelected = false;
                break;
            }
        }
        
        if (isSelected) {
            selectedVertices.push_back(vertexPos / RENDERSCALE); // Revert the global scale for accurate position
            printf("Selected vertex %zu at: (%.2f, %.2f, %.2f)\n", selectedVertices.size(), vertexPos.x, vertexPos.y, vertexPos.z);
        }
    }

    printf("Total selected: %zu vertices in drag region\n", selectedVertices.size());
    return selectedVertices;
}

Object VertexTool::CreateSelectedVerticesObject()
{
    Object obj;
    obj.color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);  // Red points
    obj.drawMode = GL_POINTS;
    obj.useIndices = false;

    // Convert selected vertex positions to vertices array
    obj.vertices.clear();
    for (const auto& vertex : selectedVertices) {
        obj.vertices.push_back(vertex.x);
        obj.vertices.push_back(vertex.y);
        obj.vertices.push_back(vertex.z);
    }
    obj.vertexCount = static_cast<uint32_t>(selectedVertices.size());

    // Create OpenGL buffers
    glGenVertexArrays(1, &obj.VAO);
    glBindVertexArray(obj.VAO);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, obj.vertices.size() * sizeof(float), obj.vertices.data(), GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    return obj;
}
