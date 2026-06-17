#pragma once
#include <vector>
#include <string>
#include <memory>
#include <limits>
#include "../modelgen/tetrahedralmeshertypes.h"

class Object;

#pragma pack(push, 1)
struct SimulationAttributes {
    float displacement[3];
    float dispMag;
    float shapeChange;
    float vonMises;
    float maxShear;
};
#pragma pack(pop)

struct SimulationStatistics {
    float minDMag = std::numeric_limits<float>::max(), maxDMag = std::numeric_limits<float>::lowest();
    float minSC  = std::numeric_limits<float>::max(), maxSC  = std::numeric_limits<float>::lowest();
    float minVM  = std::numeric_limits<float>::max(), maxVM  = std::numeric_limits<float>::lowest();
    float minMS  = std::numeric_limits<float>::max(), maxMS  = std::numeric_limits<float>::lowest();
};

static const char* SimulationVisualizationModeStrings[] = { "Displacement Magnitude", "Shape Change", "Von Mises Stress", "Max Shear Stress" };

enum class SimulationVisualizationMode {
    DisplacementMagnitude = 0,
    ShapeChange = 1,
    VonMisesStress = 2,
    MaxShearStress = 3
};

struct SimulationViewSettings {
    bool enableDisplacement = true;
    float displacementScale = 1.0f;
    SimulationVisualizationMode visualizationMode = SimulationVisualizationMode::DisplacementMagnitude;
    float minScalar = 0.0f;
    float maxScalar = 1.0f;
};

class FreeFemView {
private:
    std::vector<SimulationAttributes> simulationData;
    C3t3 simulationC3t3;
    SimulationStatistics simulationStats;
    SimulationViewSettings viewSettings;
    bool dataLoaded = false;
    std::unique_ptr<Object> renderObject;

public:
    ~FreeFemView() = default;

    SimulationStatistics getSimulationStatistics() const;
    SimulationViewSettings setViewSettings(const SimulationViewSettings& settings);
    SimulationViewSettings getViewSettings() const;
    bool loadSimulationData(const std::string& dataPath);
    bool isDataLoaded() const;
    bool loadSimulationMesh(const std::string& meshPath);
    bool loadSimulation();
    bool generateRenderObject();
    std::unique_ptr<Object>& getRenderObject();
    void clearView();
};