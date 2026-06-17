#include "modelgenui.h"
#include "../../modules/project/project.h"
#include "../../modules/gcode/gcode.h"
#include "../../modules/modelgen/layermapper.h"
#include "../../modules/modelgen/tetrahedralmesher.h"

ModelGenUI::ModelGenUI(RootUICtx* rootUICtx) : UI(rootUICtx) {
    Project* project = rootUICtx->getProject();
    LayerMapper& layerMapper = project->GetLayerMapperInstance();
    
    qualityIndex = layerMapper.nozzleQuality;
    nef_based = layerMapper.Nef_based;
}

void ModelGenUI::render() {
    ImGui::Begin("Model Generation Tools");

    RootUICtx* ctx = GetRootUIContext();
    Project* project = ctx->getProject();
    bool projectLoaded = project && project->isProjectLoaded();

    ImGui::BeginDisabled(!projectLoaded);

    // 1. Layer Mapping 
    if (ImGui::CollapsingHeader("1. Layer Mapper Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Spacing();
        ImGui::Checkbox("Use Nef-based Merging", &nef_based);
        ImGui::SliderFloat("Nozzle Diameter", &nozzleDiameter, 0.1f, 1.0f, "%.2f mm");
        
        const char* qualityItems[] = { "Low", "Medium", "High" };
        ImGui::Combo("Nozzle Quality", &qualityIndex, qualityItems, IM_ARRAYSIZE(qualityItems));
        
        ImGui::Separator();
        ImGui::Checkbox("Remesh After Layer Merging", &remesh_after_layers);
        
        if (remesh_after_layers) {
            ImGui::SliderFloat("Remesh Edge Length", &remesh_target_length, 0.1f, 5.0f, "%.2f");
            ImGui::SliderFloat("Remesh Edge Angle", &remesh_edge_angle, 25.0f, 90.0f, "%.1f°");
            ImGui::SliderInt("Remesh Iterations##remesh", &remesh_iterations, 1, 5);
        }

        ImGui::Spacing();
        if (ImGui::Button("Generate 3D Model from Layers", ImVec2(-1, 0))) {
            LayerMapper& layerMapper = project->GetLayerMapperInstance();
            layerMapper.Set2DNozzlePolygon(nozzleDiameter);
            
            switch (qualityIndex) {
                case 0:  layerMapper.nozzleQuality = LayerMapper::LOW; break;
                case 1:  layerMapper.nozzleQuality = LayerMapper::MEDIUM; break;
                case 2:  layerMapper.nozzleQuality = LayerMapper::HIGH; break;
                default: layerMapper.nozzleQuality = LayerMapper::MEDIUM; break;
            }
            
            layerMapper.Nef_based = nef_based;
            layerMapper.remesh_after_layers = remesh_after_layers;
            if (remesh_after_layers) {
                layerMapper.remesh_target_length = remesh_target_length;
                layerMapper.remesh_edge_angle = remesh_edge_angle;
                layerMapper.remesh_iterations = remesh_iterations;
            }
            project->GenerateSurfaceMesh(); 
        }
    }

    // 2. Tetrahedral Meshing
    bool hasShellMesh = projectLoaded && project->HasShellMeshGenerated();
    ImGui::BeginDisabled(!hasShellMesh);
    
    if (ImGui::CollapsingHeader("2. Tetrahedral Mesher Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Spacing();
        ImGui::SliderFloat("Cell Size", &tetrahedral_cell_size, 0.1f, 5.0f, "%.2f");
        ImGui::SliderFloat("Cell Radius Edge", &tetrahedral_cell_radius_edge, 0.1f, 5.0f, "%.2f");
        ImGui::SliderFloat("Facet Angle", &tetrahedral_facet_angle, 10.0f, 90.0f, "%.1f°");
        ImGui::SliderFloat("Facet Size", &tetrahedral_facet_size, 0.1f, 5.0f, "%.2f");
        ImGui::SliderFloat("Facet Distance", &tetrahedral_facet_distance, 0.05f, 5.0f, "%.3f");
        ImGui::SliderInt("Remesh Iterations##tetrahedral_remesh", &tetrahedral_remesh_iterations, 1, 5);

        ImGui::Spacing();
        if (ImGui::Button("Generate Tetrahedral Mesh", ImVec2(-1, 0))) {
            TetrahedralMesher& tetrahedralMesher = project->GetTetrahedralMesherInstance();
            tetrahedralMesher.cell_size        = static_cast<double>(tetrahedral_cell_size);
            tetrahedralMesher.cell_radius_edge = static_cast<double>(tetrahedral_cell_radius_edge);
            tetrahedralMesher.facet_angle      = static_cast<double>(tetrahedral_facet_angle);
            tetrahedralMesher.facet_size       = static_cast<double>(tetrahedral_facet_size);
            tetrahedralMesher.facet_distance   = static_cast<double>(tetrahedral_facet_distance);

            project->GenerateTetrahedralMesh();
        }
    }
    
    ImGui::EndDisabled();

    ImGui::EndDisabled();
    ImGui::End();
}