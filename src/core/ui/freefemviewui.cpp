#include "freefemviewui.h"

#include "../../modules/project/project.h"

FreeFemViewUI::FreeFemViewUI(RootUICtx* rootUI) : UI(rootUI) {
    viewSettings.enableDisplacement = false;
    viewSettings.displacementScale = 1.0f;
    viewSettings.visualizationMode = SimulationVisualizationMode::DisplacementMagnitude;
    viewSettings.minScalar = 0.0f;
    viewSettings.maxScalar = 1.0f;
}

void FreeFemViewUI::render() {
    RootUICtx* ctx = GetRootUIContext();
    Project* project = ctx->getProject();

    ImGui::Begin("FreeFem Simulation View");
    ImGui::BeginDisabled(!project->isProjectLoaded());
    if(ImGui::Button("Load Simulation Data")){
        project->LoadSimulationData();
    } 
    ImGui::SameLine();
    if(ImGui::Button(project->isViewResultMode() ? "Switch to Model View" : "Switch to Simulation View")){
        project->ToggleViewResultMode();
    }

    FreeFemView& freefemView = project->GetFreeFemViewInstance();

    ImGui::Text("Simulation Statistics:");
    if(freefemView.isDataLoaded()) {
        SimulationStatistics stats = freefemView.getSimulationStatistics();
        // Table View for displaying statistics
    
        ImGui::Text("Displacement Magnitude: Min = %.4f, Max = %.4f", stats.minDMag, stats.maxDMag);
        ImGui::Text("Von Mises Stress: Min = %.4f, Max = %.4f", stats.minVM, stats.maxVM);
        ImGui::Text("Max Shear Stress: Min = %.4f, Max = %.4f", stats.minMS, stats.maxMS);

        ImGui::Text("Visualization Options:");
        ImGui::Checkbox("Enable Displacement", &viewSettings.enableDisplacement);
        ImGui::SliderFloat("Displacement Scale", &viewSettings.displacementScale, 0.1f, 10.0f);
        
        int currentMode = static_cast<int>(viewSettings.visualizationMode);
        if (ImGui::Combo("Visualization Mode", &currentMode, SimulationVisualizationModeStrings, IM_ARRAYSIZE(SimulationVisualizationModeStrings))) {
            viewSettings.visualizationMode = static_cast<SimulationVisualizationMode>(currentMode);
        }
        ImGui::InputDouble("Min Scalar", &minScalar); 
        ImGui::SameLine();
        ImGui::InputDouble("Max Scalar", &maxScalar);

        viewSettings.minScalar = static_cast<float>(minScalar);
        viewSettings.maxScalar = static_cast<float>(maxScalar);
        
        if (ImGui::Button("Apply Visualization Settings")) {
            freefemView.setViewSettings(viewSettings);
        }
    } else {
        ImGui::TextWrapped("No simulation data loaded. Please load the simulation data to view statistics.");
    }
    // Additional UI controls for visualization options can be added here

    ImGui::EndDisabled();
    ImGui::End();
}