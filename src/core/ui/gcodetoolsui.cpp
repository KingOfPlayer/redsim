#include "gcodetoolsui.h"
#include "../../modules/project/project.h"
#include "../../modules/file/file.h"
#include "../../modules/gcode/gcode.h"
#include <thread>

void GCodeTools::LoadFileIntoProject(Project* project) {
    FilePath file = FileModule::SelectFile();
    if (file.path == nullptr) {
        printf("No file selected.\n");
        return;
    }
    printf("Loading GCode file into project: %s\n", file.path);
    project->LoadGCode(&file);
}

void GCodeTools::render() {
    RootUICtx* ctx = GetRootUIContext();
    Project* project = ctx->getProject();
    bool projectLoaded = project && project->isProjectLoaded();

    ImGui::Begin("GCode Tools");

    // Action execution block
    if (ImGui::CollapsingHeader("File Management", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Spacing();
        if (ImGui::Button("Load GCode File into Project", ImVec2(-1, 0))) {
            std::thread(LoadFileIntoProject, project).detach();
        }

        ImGui::BeginDisabled(!projectLoaded);
        FilePath* currentFile = projectLoaded ? project->GetCurrentGCodeFilePath() : nullptr;
        
        if (currentFile != nullptr && currentFile->path != nullptr) {
            ImGui::TextWrapped("Current File: %s", currentFile->path);
            ImGui::Spacing();
            if (ImGui::Button("Generate Render Object from GCode", ImVec2(-1, 0))) {
                project->GenerateRenderObjectFromGCode();
            }
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "No GCode File Loaded.");
        }
        ImGui::EndDisabled();
    }

    // Geometry data summary metrics block
    ImGui::BeginDisabled(!projectLoaded);
    if (ImGui::CollapsingHeader("GCode Summary", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Spacing();
        if (projectLoaded) {
            GCodeModule& gcodeModule = project->GetGCodeModuleInstance();
            
            if (gcodeModule.points.empty() || gcodeModule.paths.empty()) {
                ImGui::TextWrapped("No GCode data available. Please load a file to see parsed metrics.");
            } else {
                GCodeSummary summary = gcodeModule.GetSummary();
                
                if (ImGui::BeginTable("GCodeSummaryTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                    ImGui::TableSetupColumn("Metric Property");
                    ImGui::TableSetupColumn("Parsed Count");
                    ImGui::TableHeadersRow();

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0); ImGui::Text("Total Points");
                    ImGui::TableSetColumnIndex(1); ImGui::Text("%d", summary.totalPoints);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0); ImGui::Text("Total Paths");
                    ImGui::TableSetColumnIndex(1); ImGui::Text("%d", summary.totalPaths);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0); ImGui::Text("Total Layers");
                    ImGui::TableSetColumnIndex(1); ImGui::Text("%d", summary.totalLayers);

                    ImGui::EndTable();
                }
            }
        }
    }
    ImGui::EndDisabled();

    ImGui::End();
}