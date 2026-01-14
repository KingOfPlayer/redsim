#include "camera.h"

Camera::Camera(glm::vec3 position, glm::vec3 target, float yaw, float pitch, float distance, float sensitivity){
    this->position = position;
    this->target = target;
    this->yaw = yaw;
    this->pitch = pitch;
    this->distance = distance;
    this->sensitivity = sensitivity;
}

void Camera::Zoom(float delta) {
    distance -= delta * 0.5f;
    if (distance < 1.0f) distance = 1.0f;
}

void Camera::Pan(float deltaX, float deltaY) {
    glm::vec3 forward = glm::normalize(target - position);
    glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));
    glm::vec3 up = glm::normalize(glm::cross(right, forward));

    float panSpeed = distance * sensitivity / 100.0f; 
    glm::vec3 offset = (right * -deltaX * panSpeed) + (up * -deltaY * panSpeed);
    
    target += offset;
    position += offset;
}

void Camera::Orbit(float deltaX, float deltaY) {
    yaw   += -deltaX * sensitivity;
    pitch += deltaY * sensitivity;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;
}

glm::mat4 Camera::GetViewMatrix(float aspectRatio) {
    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction = glm::normalize(direction);

    position = target - direction * distance;

    glm::mat4 view = glm::lookAt(position, target, glm::vec3(0,1,0));
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
    glm::mat4 viewProj = proj * view;

    return viewProj;
}

