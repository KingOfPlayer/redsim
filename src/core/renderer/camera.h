#pragma once
#include <glm/glm.hpp>
#include <glm/ext.hpp>

class Camera {
glm::vec3 position;
glm::vec3 target;

float yaw;
float pitch;
float distance;
float sensitivity;
public:

    Camera(glm::vec3 position, glm::vec3 target, float yaw, float pitch, float distance, float sensitivity);

    void Zoom(float delta);
    void Orbit(float deltaX, float deltaY);
    void Pan(float deltaX, float deltaY);

    glm::mat4 GetViewMatrix(float aspectRatio);

    glm::vec3 GetPosition() const { return position; }
    glm::vec3 GetTarget() const { return target; }
};