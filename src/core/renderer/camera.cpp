#include "camera.h"
#include "renderconst.h"

Camera::Camera(glm::vec3 position, glm::vec3 target, float yaw, float pitch, float distance, float sensitivity)
{
    this->position = position;
    this->target = target;
    this->yaw = yaw;
    this->pitch = pitch;
    this->distance = distance;
    this->sensitivity = sensitivity;
}

void Camera::UpdateCameraVectors()
{
    glm::vec3 dir;
    dir.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    dir.y = sin(glm::radians(pitch));
    dir.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    
    forward = glm::normalize(dir);
    right   = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
    
    if (glm::length(right) < 0.001f) {
        right = glm::vec3(1.0f, 0.0f, 0.0f);
    }
    
    up = glm::normalize(glm::cross(right, forward));
    position = target - forward * distance;
}

void Camera::Zoom(float delta)
{
    distance -= delta * 0.5f;
    if (distance < 0.1f)
        distance = 0.1f;
}

void Camera::Pan(float deltaX, float deltaY)
{
    float panSpeed = distance * sensitivity * 0.01f;
    
    glm::vec3 offset = (right * -deltaX * panSpeed) + (up * -deltaY * panSpeed);

    target += offset;
    position += offset;
}

void Camera::Orbit(float deltaX, float deltaY)
{
    yaw   -= deltaX * sensitivity;
    pitch += deltaY * sensitivity;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;
}

glm::mat4 Camera::GetViewMatrix()
{
    UpdateCameraVectors();

    return glm::lookAt(position, target, up);
}

glm::mat4 Camera::GetProjectionMatrix(float aspectRatio)
{
    return glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
}

glm::mat4 Camera::GetViewProjectMatrix(float aspectRatio)
{
    return GetProjectionMatrix(aspectRatio) * GetViewMatrix();
}

glm::quat Camera::GetRotation()
{
    return glm::quat_cast(GetViewMatrix());
}