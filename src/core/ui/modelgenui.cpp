
#include "modelgenui.h"

void ModelGenUI::render() {
    ImGui::Begin("Model Generation Tools");

    RootUICtx* ctx = GetRootUIContext();
    Project* project = ctx->getProject();

    if(project->isProjectLoaded()){
        if(ImGui::Button("Generate 3D Model from Layers")){
            printf("Generating 3D Model from Layers...\n");
            LayerMapper& layerMapper = project->GetLayerMapper();
            layerMapper.Set2DNozzlePolygon(nozzleDiameter);
            switch (qualityIndex)
            {
            case 0:
                layerMapper.nozzleQuality = LayerMapper::LOW;
                break;
            case 1:
                layerMapper.nozzleQuality = LayerMapper::MEDIUM;
                break;
            case 2:
                layerMapper.nozzleQuality = LayerMapper::HIGH;
                break;
            default:
                layerMapper.nozzleQuality = LayerMapper::MEDIUM;
                break;
            }
            layerMapper.Nef_based = nef_based;
            layerMapper.remesh_after_layers = remesh_after_layers;
            project->Generate3DMeshFromLayers(); 
        }
    } else {
        ImGui::Text("No Project Loaded.");
    }

    // LayerMapper Options
    ImGui::Separator();
    ImGui::Text("Layer Mapper Settings:");
    ImGui::Checkbox("Use Nef-based Merging", &nef_based);
    ImGui::SliderFloat("Nozzle Diameter", &nozzleDiameter, 0.1f, 1.0f);
    // Nozzle Quality Selection
    const char* qualityItems[] = { "Low", "Medium", "High" };
    ImGui::Combo("Nozzle Quality", &qualityIndex, qualityItems, IM_ARRAYSIZE(qualityItems));
    ImGui::Checkbox("Remesh After Layer Merging", &remesh_after_layers);

    ImGui::End();
}

ModelGenUI::ModelGenUI(RootUICtx* rootUICtx) : UI(rootUICtx) {
    Project* project = rootUICtx->getProject();
    LayerMapper& layerMapper = project->GetLayerMapper();
    
    qualityIndex = layerMapper.nozzleQuality;
    nozzleDiameter = layerMapper.nozzle.diameter;
    nef_based = layerMapper.Nef_based;
}