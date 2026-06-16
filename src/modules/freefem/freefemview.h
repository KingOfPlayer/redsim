#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <map>

#include "../modelgen/tetrahedralmeshertypes.h"

#include <CGAL/IO/File_medit.h>

#include "../../core/renderer/object.h"

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
    float minSC = std::numeric_limits<float>::max(), maxSC = std::numeric_limits<float>::lowest();
    float minVM = std::numeric_limits<float>::max(), maxVM = std::numeric_limits<float>::lowest();
    float minMS = std::numeric_limits<float>::max(), maxMS = std::numeric_limits<float>::lowest();
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
    SimulationVisualizationMode visualizationMode = SimulationVisualizationMode::DisplacementMagnitude; // 0 = Displacement Magnitude, 1 = Shape Change, 2 = Von Mises, 3 = Max Shear

    float minScalar = 0.0f;
    float maxScalar = 1.0f;
};

class FreeFemView {
    std::vector<SimulationAttributes> simulationData;
    C3t3 simulationC3t3;

    SimulationStatistics simulationStats;
    SimulationViewSettings viewSettings;
    bool dataLoaded = false;

    std::unique_ptr<Object> renderObject;
public:
    ~FreeFemView() = default;

    SimulationStatistics getSimulationStatistics() const {
        return simulationStats;
    }  

    SimulationViewSettings setViewSettings(const SimulationViewSettings& settings) {
        viewSettings = settings;
        if (dataLoaded && renderObject != nullptr) {
            renderObject->setUniform("uEnableDisplacement", viewSettings.enableDisplacement ? 1 : 0);
            renderObject->setUniform("uDisplacementScale", viewSettings.displacementScale);
            renderObject->setUniform("uVisualizationMode", static_cast<int>(viewSettings.visualizationMode));
            renderObject->setUniform("uMinScalar", viewSettings.minScalar);
            renderObject->setUniform("uMaxScalar", viewSettings.maxScalar);
        }
        return viewSettings;
    }
    
    SimulationViewSettings getViewSettings() const {
        return viewSettings;
    }

    bool loadSimulationData(const std::string& dataPath) {
        if (dataPath.empty()) {
            return false;
        }

        std::ifstream inFile(dataPath);
        if (!inFile.is_open()) {
            return false;
        }

        dataLoaded = false;

        simulationData.clear();

        double dx, dy, dz;
        double dMag, sChange, vMises, mShear;

        SimulationStatistics localSimulationStats;

        localSimulationStats.minDMag = std::numeric_limits<float>::max();
        localSimulationStats.maxDMag = std::numeric_limits<float>::lowest();
        localSimulationStats.minSC   = std::numeric_limits<float>::max();
        localSimulationStats.maxSC   = std::numeric_limits<float>::lowest();
        localSimulationStats.minVM   = std::numeric_limits<float>::max();
        localSimulationStats.maxVM   = std::numeric_limits<float>::lowest();
        localSimulationStats.minMS   = std::numeric_limits<float>::max();
        localSimulationStats.maxMS   = std::numeric_limits<float>::lowest();

        while (inFile >> dx >> dy >> dz 
                    >> dMag 
                    >> sChange
                    >> vMises 
                    >> mShear) 
        {
            SimulationAttributes attr;
            
            attr.displacement[0] = static_cast<float>(dx);
            attr.displacement[1] = static_cast<float>(dy);
            attr.displacement[2] = static_cast<float>(dz);
            
            attr.dispMag  = static_cast<float>(dMag);
            attr.shapeChange = static_cast<float>(sChange);
            attr.vonMises = static_cast<float>(vMises);
            attr.maxShear = static_cast<float>(mShear);

            localSimulationStats.minDMag = std::min(localSimulationStats.minDMag, attr.dispMag);   localSimulationStats.maxDMag = std::max(localSimulationStats.maxDMag, attr.dispMag);
            localSimulationStats.minSC   = std::min(localSimulationStats.minSC, attr.shapeChange); localSimulationStats.maxSC   = std::max(localSimulationStats.maxSC, attr.shapeChange);
            localSimulationStats.minVM   = std::min(localSimulationStats.minVM, attr.vonMises);    localSimulationStats.maxVM   = std::max(localSimulationStats.maxVM, attr.vonMises);
            localSimulationStats.minMS   = std::min(localSimulationStats.minMS, attr.maxShear);    localSimulationStats.maxMS   = std::max(localSimulationStats.maxMS, attr.maxShear);

            simulationData.push_back(attr);
        }

        simulationStats = localSimulationStats;
        viewSettings.minScalar = simulationStats.minDMag;
        viewSettings.maxScalar = simulationStats.maxDMag;

        inFile.close();
        return true;
    }

    bool isDataLoaded() const {
        return dataLoaded;
    }

    bool loadSimulationMesh(const std::string& meshPath) {
        if (meshPath.empty()) {
            return false;
        }
        std::ifstream inFile(meshPath);
        CGAL::IO::read_MEDIT(inFile, simulationC3t3.triangulation());
        inFile.close();

        printf("Loaded simulation mesh from: %s\n", meshPath.c_str());
        printf("Mesh has %zu vertices and %zu cells.\n", simulationC3t3.triangulation().number_of_vertices(), simulationC3t3.triangulation().number_of_cells());
        return true;
    }

    bool isSimaulationResultReady() {
        if (simulationData.empty() || simulationC3t3.triangulation().number_of_vertices() == 0) {
            return false;
        }
        
        size_t cgalVertexCount = simulationC3t3.triangulation().number_of_vertices();
        size_t attributeCount = simulationData.size();

        if (cgalVertexCount != attributeCount) {
            return false;
        }

        dataLoaded = true;
        return true;
    }

    bool generateRenderObject() {

        if (!isSimaulationResultReady()) {
            return false;
        }

        Object renderObj;
        renderObj.drawMode = GL_TRIANGLES;
        renderObj.useIndices = true;
        renderObj.useCulling = false;

        auto& tr = simulationC3t3.triangulation();
        size_t totalVertices = tr.number_of_vertices();
        renderObj.vertexCount = totalVertices;
        
        renderObj.vertices.reserve(totalVertices * 10);

        std::map<typename C3t3::Triangulation::Vertex_handle, uint32_t> vertexToIdx;
        uint32_t currentIdx = 0;

        int vertexCounter = 0;
        int indexCounter = 0;

        for (auto vit = tr.finite_vertices_begin(); vit != tr.finite_vertices_end(); ++vit) {
            vertexToIdx[vit] = currentIdx;

            // Base Mesh Coordinates
            auto pt = vit->point();
            float posX = static_cast<float>(CGAL::to_double(pt.x()));
            float posY = static_cast<float>(CGAL::to_double(pt.y()));
            float posZ = static_cast<float>(CGAL::to_double(pt.z()));

            renderObj.vertices.push_back(posX);
            renderObj.vertices.push_back(posY);
            renderObj.vertices.push_back(posZ);

            const auto& attr = simulationData[currentIdx];

            renderObj.vertices.push_back(attr.displacement[0]);
            renderObj.vertices.push_back(attr.displacement[1]);
            renderObj.vertices.push_back(attr.displacement[2]);

            renderObj.vertices.push_back(attr.dispMag);
            renderObj.vertices.push_back(attr.shapeChange);
            renderObj.vertices.push_back(attr.vonMises);
            renderObj.vertices.push_back(attr.maxShear);

            currentIdx++;
            vertexCounter++;
        }

        for (auto cit = tr.finite_cells_begin(); cit != tr.finite_cells_end(); ++cit) {
            for (int i = 0; i < 4; ++i) {
                auto neighbor = cit->neighbor(i);
                
                if (tr.is_infinite(neighbor)) {
                    auto vh0 = cit->vertex((i + 1) % 4);
                    auto vh1 = cit->vertex((i + 2) % 4);
                    auto vh2 = cit->vertex((i + 3) % 4);

                    if (vertexToIdx.count(vh0) && vertexToIdx.count(vh1) && vertexToIdx.count(vh2)) {
                        uint32_t id0 = vertexToIdx[vh0];
                        uint32_t id1 = vertexToIdx[vh1];
                        uint32_t id2 = vertexToIdx[vh2];

                        renderObj.indices.push_back(id0);
                        renderObj.indices.push_back(id1);
                        renderObj.indices.push_back(id2);

                        indexCounter += 3;
                    }
                }
            }
        }

        printf("Generated render object with %u vertices and %zu indices.\n", renderObj.vertexCount, renderObj.indices.size());
        printf("Vertex count: %d, Index count: %d\n", vertexCounter, indexCounter);

        renderObj.setUniform("uEnableDisplacement", viewSettings.enableDisplacement ? 1 : 0);
        renderObj.setUniform("uDisplacementScale", viewSettings.displacementScale);
        renderObj.setUniform("uVisualizationMode", static_cast<int>(viewSettings.visualizationMode));

        renderObj.setUniform("uMinScalar", viewSettings.minScalar);
        renderObj.setUniform("uMaxScalar", viewSettings.maxScalar);

        glGenVertexArrays(1, &renderObj.VAO);
        glGenBuffers(1, &renderObj.VBO);
        glGenBuffers(1, &renderObj.EBO);

        glBindVertexArray(renderObj.VAO);

        glBindBuffer(GL_ARRAY_BUFFER, renderObj.VBO);
        glBufferData(GL_ARRAY_BUFFER, 
                    renderObj.vertices.size() * sizeof(float), 
                    renderObj.vertices.data(), 
                    GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderObj.EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
                    renderObj.indices.size() * sizeof(uint32_t), 
                    renderObj.indices.data(), 
                    GL_STATIC_DRAW);

        GLsizei strideBytes = 10 * sizeof(float);

        // location = 0: aPos
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, strideBytes, (void*)0);

        // location = 1: aDisp
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, strideBytes, (void*)(3 * sizeof(float)));

        // location = 2: aDispMag 
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, strideBytes, (void*)(6 * sizeof(float)));
        
        // location = 3: aShapeChange
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, strideBytes, (void*)(7 * sizeof(float)));

        // location = 4: aVonMises
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, strideBytes, (void*)(8 * sizeof(float)));

        // location = 5: aMaxShear
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, strideBytes, (void*)(9 * sizeof(float)));

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        
        renderObject = std::make_unique<Object>(renderObj);
        return true;
    }

    std::unique_ptr<Object>& getRenderObject() {
        return renderObject;
    }

    void clearView() {
        simulationData.clear();
        simulationC3t3.triangulation().clear();
        renderObject.reset();
        dataLoaded = false;
    }
};