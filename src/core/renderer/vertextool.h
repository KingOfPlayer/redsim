
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include "object.h"



class VertexTool
{
public:
    struct FrustumPlane {
        glm::vec3 normal;
        glm::vec3 cameraPos;
        
        bool isPointInFront(const glm::vec3& point) const {
            return glm::dot(normal, point - cameraPos) >= 0.0f;
        }
    };

    static std::vector<glm::vec3> selectedVertices;  // Store selected vertex positions

    static glm::vec3 GetWorldRayFromScreen(float x, float y, const glm::mat4& projection, const glm::mat4& view);

    static std::vector<glm::vec3> SelectVertices(const std::unique_ptr<Object>& object, glm::vec2 start, glm::vec2 end, const glm::mat4& proj, const glm::mat4& view, const glm::vec3& cameraPos);
    
    static Object CreateSelectedVerticesObject();
};