#pragma once
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <cassert>
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
	VertexGroupBaseType(int id, std::vector<glm::vec3> points) : labelID(id), points(points) {}
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

public:
	FixedVertexGroupType(int id, std::vector<glm::vec3> points, glm::vec3 val = glm::vec3(0.0f))
			: VertexGroupBaseType(id, points), fixedValue(val) {}

	VertexGroupLabelType getLabelType() const override
	{
		return VertexGroupLabelType::Fixed;
	}

	std::string generateBoundaryString() const
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

	std::string generateStiffnessPart() const override
	{
		return generateBoundaryString();
	}

	std::string generateRhsPart() const override
	{
		return generateBoundaryString();
	}
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
	ForceVertexGroupType(int id, std::vector<glm::vec3> points, double fVal, ForceDirection dir)
			: VertexGroupBaseType(id, points), direction(dir)
	{
		std::stringstream stream;
		stream << std::fixed << std::setprecision(2) << fVal;
		forceValue = stream.str();
	}

	VertexGroupLabelType getLabelType() const override
	{
		return VertexGroupLabelType::Force;
	}

	std::string generateStiffnessPart() const override
	{
		return "";
	}

	std::string generateRhsPart() const override
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
};

enum class FreeFemStatus {
    Idle,
    Running,
    Success,
    Failed,
	Aborted 
};