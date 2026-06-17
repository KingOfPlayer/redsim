#include "freefemui.h"
#include "../../modules/project/project.h"
#include "../../modules/freefem/freefemscript.h"
#include "../../modules/freefem/freefem.h"

void FreeFemUI::render() {
    RootUICtx* ctx = GetRootUIContext();
    Project* project = ctx->getProject();
    bool projectLoaded = project && project->isProjectLoaded();

    ImGui::Begin("FreeFEM Simulation Tools");
    ImGui::BeginDisabled(!projectLoaded);

    // Material parameters block
    if (ImGui::CollapsingHeader("Material Properties", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Spacing();
        ImGui::InputDouble("Young's Modulus (MPa)", &EValue, 10.0, 100.0, "%.2f");
        ImGui::InputDouble("Poisson's Ratio", &PoissonRatioValue, 0.01, 0.05, "%.3f");
        
        ImGui::Spacing();
        if (ImGui::Button("Generate FreeFEM Script", ImVec2(-1, 0))) {

            FreeFemScript& freefemScript = project->GetFreeFemScriptInstance();
            std::string baseDir = project->GetFileDirectory() + "/" + project->GetFilenameWithoutExtension();
            
            freefemScript.setMaterialProperties(EValue, PoissonRatioValue);
            freefemScript.setScriptPath(baseDir + "_simulation.edp");
            freefemScript.setMeshFilePath(baseDir + "_tetrahedral.mesh");
            freefemScript.setScriptOutputPath(baseDir + "_simulation_data.txt");
            
            if (EValue <= 0.0 || PoissonRatioValue < 0.0 || PoissonRatioValue >= 0.5 || freefemScript.GenerateScript()) {
                ImGui::OpenPopup("Failed Generate Script");
            } else {
                ImGui::OpenPopup("Generated Script");
            }
        }
    }

    // Process monitoring block
    FreeFemModule& freefemModule = project->GetFreeFemModuleInstance();
    FreeFemStatus status = freefemModule.GetStatus();
    
    if (ImGui::CollapsingHeader("Simulation Controls", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Spacing();
        
        if (ImGui::Button("Run FreeFEM Simulation", ImVec2(ImGui::GetWindowWidth() * 0.5f - 10.0f, 0))) {
            std::string scriptPath = project->GetFileDirectory() + "/" + project->GetFilenameWithoutExtension() + "_simulation.edp";
            freefemModule.StartSimulation(scriptPath);
        }
        ImGui::SameLine();
        
        ImGui::BeginDisabled(status != FreeFemStatus::Running);
        if (ImGui::Button("Abort Simulation", ImVec2(-1, 0))) {
            freefemModule.AbortSimulation();
        }
        ImGui::EndDisabled();

        const char* statusStr = "Unknown";
        switch (status) {
            case FreeFemStatus::Idle:    statusStr = "Idle"; break;
            case FreeFemStatus::Running: statusStr = "Running"; break;
            case FreeFemStatus::Success: statusStr = "Success"; break;
            case FreeFemStatus::Failed:  statusStr = "Failed"; break;
            case FreeFemStatus::Aborted: statusStr = "Aborted"; break;
        }
        ImGui::Text("Current FreeFEM Status: %s", statusStr);
    }

    // Diagnostics engine logging block
    if (ImGui::CollapsingHeader("FreeFEM Output Log", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Spacing();
        if (ImGui::BeginChild("FreeFEMOutputLog", ImVec2(0, 150), true)) {
            ImGui::TextWrapped("%s", freefemModule.GetOutputLog().c_str());
        }
        ImGui::EndChild();
    }

    ImGui::EndDisabled();

    // Validation warning dialog
    if (ImGui::BeginPopupModal("Invalid Input", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Invalid material values found.\nEnsure E > 0 and 0 <= Poisson < 0.5");
        ImGui::Spacing();
        if (ImGui::Button("Close", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }

    // Failed Generate Script Popup
    if (ImGui::BeginPopupModal("Failed Generate Script", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text(" Failed to generate FreeFEM script. Please check the input parameters and try again.");
        ImGui::TextWrapped(" Ensure;\n- E > 0\n- 0 <= Poisson < 0.5\n- Valid vertex groups are added");  
        ImGui::Spacing();
        if (ImGui::Button("Close", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("Generated Script", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("FreeFEM script generated successfully.");  
        ImGui::Spacing();
        if (ImGui::Button("Close", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }

    ImGui::End();
}