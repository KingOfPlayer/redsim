#pragma once
#include <glm/glm.hpp>
#include <vector>

class Project;

class RootUICtx {
    Project* project;
    std::optional<std::vector<glm::vec3>> selectedVertices;

public:
    RootUICtx(Project* proj) : project(proj){};
    ~RootUICtx();
    Project* getProject();

    void SetSelectedVertices(const std::vector<glm::vec3>& vertices);
    std::optional<std::vector<glm::vec3>> GetSelectedVertices() const;
    void ClearSelectedVertices();
};