
#include "object.h"

glm::mat4 Object::GetModelMatrix() {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    // Apply rotations
    model = glm::rotate(model, glm::radians(rotation.x), {1, 0, 0});
    model = glm::rotate(model, glm::radians(rotation.y), {0, 1, 0});
    model = glm::rotate(model, glm::radians(rotation.z), {0, 0, 1});
    model = glm::scale(model, scale);
    return model;
}

