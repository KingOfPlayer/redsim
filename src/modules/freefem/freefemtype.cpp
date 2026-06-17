#include "freefemtype.h"
#include <sstream>
#include <iomanip>
#include <cassert>

VertexGroupBaseType::VertexGroupBaseType(int id, std::vector<glm::vec3> points) 
    : labelID(id), points(points) {}

FixedVertexGroupType::FixedVertexGroupType(int id, std::vector<glm::vec3> points, glm::vec3 val)
    : VertexGroupBaseType(id, points), fixedValue(val) {}

VertexGroupLabelType FixedVertexGroupType::getLabelType() const
{
    return VertexGroupLabelType::Fixed;
}

std::string FixedVertexGroupType::generateBoundaryString() const
{
    if (points.empty())
    {
        return "";
    }
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2);
    ss << " on(" << labelID + LABEL_ID_OFFSET
       << ", ux=" << fixedValue.x
       << ", uy=" << fixedValue.y
       << ", uz=" << fixedValue.z << ")\n";
    return ss.str();
}

std::string FixedVertexGroupType::generateStiffnessPart() const
{
    return generateBoundaryString();
}

std::string FixedVertexGroupType::generateRhsPart() const
{
    return generateBoundaryString();
}

ForceVertexGroupType::ForceVertexGroupType(int id, std::vector<glm::vec3> points, double fVal, ForceDirection dir)
    : VertexGroupBaseType(id, points), direction(dir)
{
    std::stringstream stream;
    stream << std::fixed << std::setprecision(2) << fVal;
    forceValue = stream.str();
}

VertexGroupLabelType ForceVertexGroupType::getLabelType() const
{
    return VertexGroupLabelType::Force;
}

std::string ForceVertexGroupType::generateStiffnessPart() const
{
    return "";
}

std::string ForceVertexGroupType::generateRhsPart() const
{
    if (points.empty())
    {
        return "";
    }
    std::string forceDirectionStr;
    switch (direction)
    {
    case ForceDirection::X:
        forceDirectionStr = "vx";
        break;
    case ForceDirection::Y:
        forceDirectionStr = "vy";
        break;
    case ForceDirection::Z:
        forceDirectionStr = "vz";
        break;
    default:
        assert(false && "Invalid force direction");
        break;
    }

    return "int2d(Th, " + std::to_string(labelID + LABEL_ID_OFFSET) + ")( " + forceValue + "*" + forceDirectionStr + " )\n";
}