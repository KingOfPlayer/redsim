
#include "../../modules/gcode/gcode.h"
#include "../../modules/modelgen/layermaper.h"
#include "rootui.h"
#include <vector>
#pragma once


class ModelGenUI: public UI{
    float nozzleDiameter = 0.46f;
    int qualityIndex = 0;
    void render() override {
        ImGui::Begin("Model Generation Tools");

        RootUICtx* ctx = GetRootUIContext();
        Project* project = ctx->getProject();

        if(project->isProjectLoaded()){
            if(ImGui::Button("Generate 3D Model from Layers")){
                printf("Generating 3D Model from Layers...\n");
                project->GetLayerMapper().Set2DNozzlePolygon(nozzleDiameter);
                switch (qualityIndex)
                {
                case 0:
                    project->GetLayerMapper().nozzleQuality = LayerMapper::LOW;
                    break;
                case 1:
                    project->GetLayerMapper().nozzleQuality = LayerMapper::MEDIUM;
                    break;
                case 2:
                    project->GetLayerMapper().nozzleQuality = LayerMapper::HIGH;
                    break;
                default:
                    project->GetLayerMapper().nozzleQuality = LayerMapper::MEDIUM;
                    break;
                }
                project->Generate3DMeshFromLayers(); 
            }
        } else {
            ImGui::Text("No Project Loaded.");
        }

        // LayerMapper Options
        ImGui::Separator();
        ImGui::Text("Layer Mapper Settings:");
        ImGui::SliderFloat("Nozzle Diameter", &nozzleDiameter, 0.1f, 1.0f);
        // Nozzle Quality Selection
        const char* qualityItems[] = { "Low", "Medium", "High" };
        ImGui::Combo("Nozzle Quality", &qualityIndex, qualityItems, IM_ARRAYSIZE(qualityItems));



        if (ImGui::Button("Run Test Sample Model")){
            // Nozzle2D nozzle = LayerMapper::Generate2DNozzlePolygon(0.4f, LayerMapper::MEDIUM);
            // printf("Generated Nozzle Polygon with %zu points.\n", nozzle.polygon.size());

            // // Square test points and paths
            // std::vector<GCodePoint> testPoints = {
            //     {2.0f, 2.0f, 0.0f},
            //     {2.0f, -2.0f, 0.0f},
            //     {-2.0f, -2.0f, 0.0f},
            //     {-2.0f, 2.0f, 0.0f},
            // };
            // GCodePath path1;
            // path1.start = 0;
            // path1.end = 1;
            // // GCodePath path2;
            // // path2.start = 1;
            // // path2.end = 2;
            // GCodePath path3;
            // path3.start = 2;
            // path3.end = 3;
            // // GCodePath path4;
            // // path4.start = 3;
            // // path4.end = 0;
            // std::vector<GCodePath> testPaths = {path1, /*path2,*/ path3 /*, path4*/};
            // std::list<Polygon_with_holes_2> layer = LayerMapper::GenerateLayerFromPaths(nozzle, testPoints, testPaths);
            
            // //LayerMapper::Extrude2DLayerTo3D(layer, 0.2f);
        }

        ImGui::End();
    }
    public:
    ModelGenUI(RootUICtx* rootUICtx) : UI(rootUICtx) {
        Project* project = rootUICtx->getProject();
        LayerMapper& layerMapper = project->GetLayerMapper();
        
        qualityIndex = layerMapper.nozzleQuality;
        nozzleDiameter = layerMapper.nozzle.diameter;
    }
};