#pragma once
#include <vector>
#include <string>
#include <glm/glm.hpp>

const int LABEL_ID_OFFSET = 2;

static const char* VertexGroupLabelTypeStrings[] = { "Fixed", "Force" };

enum class VertexGroupLabelType
{
    Fixed,
    Force
};

class VertexGroupBaseType
{
protected:
    std::vector<glm::vec3> points;
    int labelID;

public:
    VertexGroupBaseType(int id, std::vector<glm::vec3> points);
    virtual ~VertexGroupBaseType() = default;

    virtual VertexGroupLabelType getLabelType() const = 0;
    virtual std::string generateStiffnessPart() const = 0;
    virtual std::string generateRhsPart() const = 0;

    int getLabelID() const { return labelID; }
    const std::vector<glm::vec3>& getPoints() const { return points; }
};

class FixedVertexGroupType : public VertexGroupBaseType
{
private:
    glm::vec3 fixedValue;
    std::string generateBoundaryString() const;

public:
    FixedVertexGroupType(int id, std::vector<glm::vec3> points, glm::vec3 val = glm::vec3(0.0f));

    VertexGroupLabelType getLabelType() const override;
    std::string generateStiffnessPart() const override;
    std::string generateRhsPart() const override;
};

static const char* ForceDirectionStrings[] = { "X", "Y", "Z" };

enum class ForceDirection
{
    X,
    Y,
    Z
};

class ForceVertexGroupType : public VertexGroupBaseType
{
private:
    std::string forceValue;
    ForceDirection direction;

public:
    ForceVertexGroupType(int id, std::vector<glm::vec3> points, double fVal, ForceDirection dir);

    VertexGroupLabelType getLabelType() const override;
    std::string generateStiffnessPart() const override;
    std::string generateRhsPart() const override;
};

enum class FreeFemStatus {
    Idle,
    Running,
    Success,
    Failed,
    Aborted 
};