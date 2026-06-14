#pragma once
#include <glm/glm.hpp>
#include <glm/ext.hpp>

class Camera {
glm::vec3 position;
glm::vec3 target;
glm::vec3 forward;
glm::vec3 right;
glm::vec3 up;

float yaw;
float pitch;
float distance;
float sensitivity;
public:

    Camera(glm::vec3 position, glm::vec3 target, float yaw, float pitch, float distance, float sensitivity);

    void UpdateCameraVectors();
    void Zoom(float delta);
    void Orbit(float deltaX, float deltaY);
    void Pan(float deltaX, float deltaY);

    glm::mat4 GetViewMatrix();
    glm::mat4 GetProjectionMatrix(float aspectRatio);
    glm::mat4 GetViewProjectMatrix(float aspectRatio);

    glm::vec3 GetPosition() const { return position; }
    glm::quat GetRotation();
    glm::vec3 GetTarget() const { return target; }
};