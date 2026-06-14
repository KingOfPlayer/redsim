#include "freefemui.h"

#include "../../modules/project/project.h"
#include "../../modules/freefem/freefemscript.h"
#include "../../modules/freefem/freefem.h"

void FreefemUI::render(){
    RootUICtx* ctx = GetRootUIContext();
    Project* project = ctx->getProject();

    ImGui::Begin("FreeFEM Simulation Tools");

    ImGui::Text("Material Properties:");
    ImGui::InputDouble("Young's Modulus (Mpa)", &EValue);
    ImGui::InputDouble("Poisson's Ratio", &PoissonRatioValue);
    ImGui::Separator();
    
    if(ImGui::Button("Generate FreeFEM Script")){
        FreeFemScript& freefemScript = project->GetFreeFemScriptInstance();

        freefemScript.setMaterialProperties(EValue, PoissonRatioValue);
        freefemScript.setScriptPath(project->GetFileDirectory() + "/" + project->GetFilenameWithoutExtension() + "_simulation.edp");
        freefemScript.GenerateScript();
    }

     FreeFemStatus status = freefemModule.GetStatus();
    std::string statusStr;
    switch (status) {
        case FreeFemStatus::Idle: statusStr = "Idle"; break;
        case FreeFemStatus::Running: statusStr = "Running"; break;
        case FreeFemStatus::Success: statusStr = "Success"; break;
        case FreeFemStatus::Failed: statusStr = "Failed"; break;
        case FreeFemStatus::Aborted: statusStr = "Aborted"; break;
    }

    FreeFemModule& freefemModule = project->GetFreeFemModuleInstance();
    if(ImGui::Button("Run FreeFEM Simulation")){
        std::string scriptPath = project->GetFileDirectory() + "/" + project->GetFilenameWithoutExtension() + "_simulation.edp";
        freefemModule.StartSimulation(scriptPath);
    }

    ImGui::beginDisabled(status != FreeFemStatus::Running);
    if(ImGui::Button("Abort Simulation")){
        freefemModule.AbortSimulation();
    }
    ImGui::endDisabled();
   
    ImGui::Text("Current Freefem Status: %s", statusStr.c_str());

    // Display FreeFEM output log
    ImGui::Separator();
    ImGui::Text("FreeFEM Output Log:");
    ImGui::BeginChild("FreeFEMOutputLog", ImVec2(0, 200), true);
    ImGui::TextWrapped("%s", freefemModule.GetOutputLog().c_str());
    ImGui::EndChild();

    ImGui::End();
}