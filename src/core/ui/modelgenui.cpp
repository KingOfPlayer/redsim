
#include "modelgenui.h"

#include "../../modules/project/project.h"
#include "../../modules/gcode/gcode.h"
#include "../../modules/modelgen/layermapper.h"
#include "../../modules/modelgen/tetrahedralmesher.h"

void ModelGenUI::render() {
    ImGui::Begin("Model Generation Tools");

    RootUICtx* ctx = GetRootUIContext();
    Project* project = ctx->getProject();

    bool projectLoaded = project->isProjectLoaded();
    if(projectLoaded) {
        if(ImGui::Button("Generate 3D Model from Layers")){
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
            if(remesh_after_layers){
                layerMapper.remesh_target_length = remesh_target_length;
                layerMapper.remesh_edge_angle = remesh_edge_angle;
                layerMapper.remesh_iterations = remesh_iterations;
            }
            project->GenerateShellMesh(); 
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
    ImGui::Separator();
    ImGui::Checkbox("Remesh After Layer Merging", &remesh_after_layers);
    if(remesh_after_layers) {
        ImGui::SliderFloat("Remesh Edge Length", &remesh_target_length, 0.1f, 5.0f);
        ImGui::SliderFloat("Remesh Edge Angle", &remesh_edge_angle, 25.0f, 90.0f);
        ImGui::SliderInt("Remesh Iterations", &remesh_iterations, 1, 5);
        ImGui::Separator();
    }

    if(project->HasShellMeshGenerated()) {
        ImGui::Separator();
        if(ImGui::Button("Generate Tetrahedral Mesh")){
            TetrahedralMesher& tetrahedralMesher = project->GetTetrahedralMesher();
            tetrahedralMesher.cell_size        = (double)tetrahedral_cell_size       ;
            tetrahedralMesher.cell_radius_edge = (double)tetrahedral_cell_radius_edge;
            tetrahedralMesher.facet_angle      = (double)tetrahedral_facet_angle     ;
            tetrahedralMesher.facet_size       = (double)tetrahedral_facet_size      ;
            tetrahedralMesher.facet_distance   = (double)tetrahedral_facet_distance  ;

            project->GenerateTetrahedralMesh();
        }
        ImGui::Text("Tetrahedral Mesher Settings:");
        ImGui::SliderFloat("Tetrahedral Cell Size", &tetrahedral_cell_size, 0.1f, 5.0f);
        ImGui::SliderFloat("Tetrahedral Cell Radius Edge", &tetrahedral_cell_radius_edge, 0.1f, 5.0f);
        ImGui::SliderFloat("Tetrahedral Facet Angle", &tetrahedral_facet_angle, 10.0f, 90.0f);
        ImGui::SliderFloat("Tetrahedral Facet Size", &tetrahedral_facet_size, 0.1f, 5.0f);
        ImGui::SliderFloat("Tetrahedral Facet Distance", &tetrahedral_facet_distance, 0.05f, 5.0f);
        ImGui::SliderInt("Tetrahedral Remesh Iterations", &tetrahedral_remesh_iterations, 1, 5);

	double cell_size        = 2.0;
	double cell_radius_edge = 2.0;
	double facet_angle      = 25.0;
	double facet_size       = 2.0;
	double facet_distance   = 0.05;
	int    remesh_iterations = 1;  

        if(project->HasTetrahedralMeshGenerated() && ImGui::Button("Save Tetrahedral Mesh")){
            project->SaveTetrahedralMeshToFile();
        }
    }
    

    ImGui::End();
}

ModelGenUI::ModelGenUI(RootUICtx* rootUICtx) : UI(rootUICtx) {
    Project* project = rootUICtx->getProject();
    LayerMapper& layerMapper = project->GetLayerMapper();
    
    qualityIndex = layerMapper.nozzleQuality;
    nozzleDiameter = layerMapper.nozzle.diameter;
    nef_based = layerMapper.Nef_based;
    remesh_after_layers = layerMapper.remesh_after_layers;
}