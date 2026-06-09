#include "rootuictx.h"

#include "../../modules/project/project.h"

RootUICtx::~RootUICtx() {
    delete project;
}

Project* RootUICtx::getProject() {
    return project;
}


void RootUICtx::SetSelectedVertices(const std::vector<glm::vec3>& vertices) {
    selectedVertices = vertices; // Automatically defines and populates the optional
}

std::optional<std::vector<glm::vec3>> RootUICtx::GetSelectedVertices() const {
    return selectedVertices; // Returns the optional wrapper
}

void RootUICtx::ClearSelectedVertices() {
    selectedVertices = std::nullopt; // Resets it back to "not defined"
}