
#include "object.h"

glm::mat4 Object::GetModelMatrix() {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    // Apply rotations
    model = glm::rotate(model, glm::radians(rotation.x), {1, 0, 0});
    model = glm::rotate(model, glm::radians(rotation.y), {0, 1, 0});
    model = glm::rotate(model, glm::radians(rotation.z), {0, 0, 1});
    glm::vec3 correct_scale = scale;
    correct_scale.y *= -1; // Invert Y scale to correct for coordinate system
    model = glm::scale(model, correct_scale);
    return model;
}

