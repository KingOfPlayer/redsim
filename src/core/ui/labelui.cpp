#include "labelui.h"
#include "../../modules/project/project.h"
#include "../../modules/freefem/freefemscript.h"

LabelUI::LabelUI(RootUICtx* rootUICtx) : UI(rootUICtx) {}

void LabelUI::render() {
    ImGui::Begin("Finite Element Analysis Labels");

    RootUICtx* ctx = GetRootUIContext();
    Project* project = ctx->getProject();
    bool hasMesh = project && project->HasTetrahedralMeshGenerated();

    ImGui::BeginDisabled(!hasMesh);

    // Vertex selection info
    auto selectedVertices = ctx->GetSelectedVertices();
    if (selectedVertices.has_value()) {
        std::vector<glm::vec3>& vertices = selectedVertices.value();
        ImGui::Text("%zu Selected Vertices", vertices.size());
    } else {
        ImGui::Text("No Vertices Selected");
    }
    
    // Management block
    if (ImGui::CollapsingHeader("Vertex Group Management", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Spacing();
        ImGui::InputInt("Label Index", &new_vertex_label);
        ImGui::Combo("Vertex Type", &new_vertex_type_index, VertexGroupLabelTypeStrings, IM_ARRAYSIZE(VertexGroupLabelTypeStrings));
        
        if (new_vertex_type_index == 1) {
            ImGui::Combo("Force Direction", &new_froceDirection_index, ForceDirectionStrings, IM_ARRAYSIZE(ForceDirectionStrings));
            ImGui::InputInt("Force MPa", &new_forceValue);
        }

        ImGui::Spacing();
        if (ImGui::Button("Add Vertex Group", ImVec2(-1, 0))) {
            if (selectedVertices.has_value() && !selectedVertices->empty()) {
                bool labelExists = false;
                for (const auto& group : groups) {
                    if (group->getLabelID() == new_vertex_label) {
                        ImGui::OpenPopup("SameLabelPopup");
                        labelExists = true;
                        break;
                    }
                }
                if (!labelExists) {
                    std::vector<glm::vec3>& vertices = selectedVertices.value();
                    int id = new_vertex_label;
                    if (new_vertex_type_index == 0) {
                        groups.push_back(std::make_unique<FixedVertexGroupType>(id, vertices));
                    } else if (new_vertex_type_index == 1) {
                        ForceDirection dir = static_cast<ForceDirection>(new_froceDirection_index);
                        groups.push_back(std::make_unique<ForceVertexGroupType>(id, vertices, new_forceValue, dir));
                    }
                    new_vertex_label++;
                }
            }
        }
    }

    // List view block
    if (ImGui::CollapsingHeader("Existing Vertex Groups", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Spacing();
        if (ImGui::BeginChild("VertexGroupList", ImVec2(0, 150), true)) {
            for (auto it = groups.begin(); it != groups.end(); ) {
                const auto& group = *it;
                std::string labelTypeStr = VertexGroupLabelTypeStrings[static_cast<int>(group->getLabelType())];
                std::string detailsStr = "ID: " + std::to_string(group->getLabelID()) + " | Type: " + labelTypeStr + " | Verts: " + std::to_string(group->getPoints().size());
                
                ImGui::TextUnformatted(detailsStr.c_str());
                ImGui::SameLine(ImGui::GetWindowWidth() - 75);

                std::string removeButtonLabel = "Remove##" + std::to_string(group->getLabelID());
                if (ImGui::Button(removeButtonLabel.c_str())) {
                    it = groups.erase(it);
                } else {
                    ++it;
                }
            }
        }
        ImGui::EndChild();
    }

    // Actions block
    if (ImGui::CollapsingHeader("Pipeline Actions", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Spacing();
        ImGui::BeginDisabled(groups.empty());
        if (ImGui::Button("Label Apply to Mesh", ImVec2(-1, 0))) {
            if (groups.empty()) {
                ImGui::OpenPopup("NoGroupsPopup");
            } else {
                project->ApplyLabel(std::move(groups));
            }
        }
        ImGui::EndDisabled();
        
        ImGui::Spacing();
        if (ImGui::Button("Save Tetrahedral Mesh", ImVec2(-1, 0))) {
            project->SaveTetrahedralMeshToFile();
        }
    }

    ImGui::EndDisabled();

    // Popups
    if (ImGui::BeginPopupModal("SameLabelPopup", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Same label index already exists.\nPlease choose a different index.");
        ImGui::Spacing();
        if (ImGui::Button("Close", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("NoGroupsPopup", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("No vertex groups to label.\nPlease add at least one vertex group.");
        ImGui::Spacing();
        if (ImGui::Button("Close", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }

    ImGui::End();
}