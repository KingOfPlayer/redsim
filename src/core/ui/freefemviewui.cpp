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
    bool projectLoaded = project && project->isProjectLoaded();

    ImGui::Begin("FreeFem Simulation View");
    ImGui::BeginDisabled(!projectLoaded);

    // Primary workflow action buttons
    if (ImGui::Button("Load Simulation Data", ImVec2(ImGui::GetWindowWidth() * 0.5f - 10.0f, 0))) {
        if(project->LoadSimulationData()){
            FreeFemView& freefemView = project->GetFreeFemViewInstance();
            SimulationViewSettings currentSettings = freefemView.getViewSettings();
            viewSettings = currentSettings;
            minScalar = static_cast<double>(currentSettings.minScalar);
            maxScalar = static_cast<double>(currentSettings.maxScalar);
        }
    }
    ImGui::SameLine();
    
    const char* toggleText = project && project->isViewResultMode() ? "Switch to Model View" : "Switch to Simulation View";
    if (ImGui::Button(toggleText, ImVec2(-1, 0))) {
        project->ToggleViewResultMode();
    }

    FreeFemView& freefemView = project->GetFreeFemViewInstance();

    if (freefemView.isDataLoaded()) {
        SimulationStatistics stats = freefemView.getSimulationStatistics();

        // Metric bounds summary container
        if (ImGui::CollapsingHeader("Simulation Statistics", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Spacing();
            if (ImGui::BeginTable("StatsTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                ImGui::TableSetupColumn("Metric");
                ImGui::TableSetupColumn("Min Value");
                ImGui::TableSetupColumn("Max Value");
                ImGui::TableHeadersRow();

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0); ImGui::Text("Displacement Magnitude");
                ImGui::TableSetColumnIndex(1); ImGui::Text("%.4f", stats.minDMag);
                ImGui::TableSetColumnIndex(2); ImGui::Text("%.4f", stats.maxDMag);

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0); ImGui::Text("Shape Change");
                ImGui::TableSetColumnIndex(1); ImGui::Text("%.4f", stats.minSC);
                ImGui::TableSetColumnIndex(2); ImGui::Text("%.4f", stats.maxSC);

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0); ImGui::Text("Von Mises Stress");
                ImGui::TableSetColumnIndex(1); ImGui::Text("%.4f", stats.minVM);
                ImGui::TableSetColumnIndex(2); ImGui::Text("%.4f", stats.maxVM);

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0); ImGui::Text("Max Shear Stress");
                ImGui::TableSetColumnIndex(1); ImGui::Text("%.4f", stats.minMS);
                ImGui::TableSetColumnIndex(2); ImGui::Text("%.4f", stats.maxMS);

                ImGui::EndTable();
            }
        }

        // Geometry presentation modifiers
        if (ImGui::CollapsingHeader("Deformation Parameters", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Spacing();
            ImGui::Checkbox("Enable Displacement Map", &viewSettings.enableDisplacement);
            ImGui::SliderFloat("Scale Factor", &viewSettings.displacementScale, 0.1f, 10.0f, "%.1fx");
        }
        
        if (ImGui::CollapsingHeader("Heatmap Configuration", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Spacing();
            int currentMode = static_cast<int>(viewSettings.visualizationMode);
            if (ImGui::Combo("Target Scalar Field", &currentMode, SimulationVisualizationModeStrings, IM_ARRAYSIZE(SimulationVisualizationModeStrings))) {
                viewSettings.visualizationMode = static_cast<SimulationVisualizationMode>(currentMode);

                switch (viewSettings.visualizationMode) {
                    case SimulationVisualizationMode::DisplacementMagnitude:
                        minScalar = static_cast<double>(stats.minDMag);
                        maxScalar = static_cast<double>(stats.maxDMag);
                        break;
                    case SimulationVisualizationMode::ShapeChange:
                        minScalar = static_cast<double>(stats.minSC);
                        maxScalar = static_cast<double>(stats.maxSC);
                        break;
                    case SimulationVisualizationMode::VonMisesStress:
                        minScalar = static_cast<double>(stats.minVM);
                        maxScalar = static_cast<double>(stats.maxVM);
                        break;
                    case SimulationVisualizationMode::MaxShearStress:
                        minScalar = static_cast<double>(stats.minMS);
                        maxScalar = static_cast<double>(stats.maxMS);
                        break; 
                }
            }

            ImGui::InputDouble("Scalar Minimum", &minScalar, 0.01, 0.1, "%.4f"); 
            ImGui::InputDouble("Scalar Maximum", &maxScalar, 0.01, 0.1, "%.4f");

            viewSettings.minScalar = static_cast<float>(minScalar);
            viewSettings.maxScalar = static_cast<float>(maxScalar);
            
            // Color Map
            ImGui::Text("Color Map:");
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            ImVec2 pMin = ImGui::GetCursorScreenPos();
            float availableWidth = ImGui::GetContentRegionAvail().x;
            ImVec2 pMax = ImVec2(pMin.x + availableWidth, pMin.y + 18.0f);
            
            ImGui::InvisibleButton("##GradientPreview", ImVec2(availableWidth, 18.0f));
            
            // Render Color Gradient
            const int steps = 64;
            float stepWidth = availableWidth / steps;
            for (int i = 0; i < steps; ++i) {
                float v = static_cast<float>(i) / (steps - 1);
                float r = glm::clamp(1.5f - glm::abs(v * 4.0f - 3.0f), 0.0f, 1.0f);
                float g = glm::clamp(1.5f - glm::abs(v * 4.0f - 2.0f), 0.0f, 1.0f);
                float b = glm::clamp(1.5f - glm::abs(v * 4.0f - 1.0f), 0.0f, 1.0f);
                
                ImU32 col = ImGui::ColorConvertFloat4ToU32(ImVec4(r, g, b, 1.0f));
                drawList->AddRectFilled(ImVec2(pMin.x + i * stepWidth, pMin.y), ImVec2(pMin.x + (i + 1) * stepWidth, pMax.y), col);
            }
            ImGui::Spacing();

            // Render 5 text
            for (int i = 0; i < 5; ++i) {
                float fraction = static_cast<float>(i) / 4.0f;
                double currentValue = viewSettings.minScalar + fraction * (viewSettings.maxScalar - viewSettings.minScalar);
                
                char labelBuf[32];
                snprintf(labelBuf, sizeof(labelBuf), "%.2e", currentValue);
                
                float textWidth = ImGui::CalcTextSize(labelBuf).x;
                float labelX = pMin.x + (fraction * availableWidth) - (fraction * textWidth);
                
                ImGui::SetCursorScreenPos(ImVec2(labelX, ImGui::GetCursorScreenPos().y));
                ImGui::TextUnformatted(labelBuf);
                
                if (i < 4) ImGui::SameLine();
            }
            ImGui::Spacing();

        }

        if (ImGui::Button("Apply Visualization Settings", ImVec2(-1, 0))) {
            freefemView.setViewSettings(viewSettings);
        }
    } else {
        ImGui::TextWrapped("No simulation data loaded. Please load the simulation data to view statistics.");
    }

    ImGui::EndDisabled();
    ImGui::End();
}