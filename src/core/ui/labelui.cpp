#include "labelui.h"


#include "../../modules/project/project.h"
#include "../../modules/freefem/freefemscript.h"

void LabelUI::render() {
    ImGui::Begin("Finite Element Analysis Labels");

    RootUICtx* ctx = GetRootUIContext();
    Project* project = ctx->getProject();

    auto selectedVertices = ctx->GetSelectedVertices();

    if(selectedVertices.has_value()) {
      std::vector<glm::vec3>& vertices = selectedVertices.value();
      ImGui::Text("%li Selected Vertices", vertices.size());
    } else {
      ImGui::Text("No Vertices Selected");
    }
    
    // Add Vertex Group Management UI
    ImGui::Separator();
    ImGui::Text("Vertex Group Management:");
    ImGui::InputInt("Label Index", &new_vertex_label);
    ImGui::Combo("Nozzle Quality", &new_vertex_type_index, VertexGroupLabelTypeStrings, IM_ARRAYSIZE(VertexGroupLabelTypeStrings));
    if (new_vertex_type_index == 1) {
        ImGui::Combo("Force Direction", &new_froceDirection_index, ForceDirectionStrings, IM_ARRAYSIZE(ForceDirectionStrings));
        ImGui::InputInt("Force Value", &new_forceValue);
    }
    if (ImGui::Button("Add Vertex Group")) {
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
              if (new_vertex_type_index == 0) { // Fixed
                  groups.push_back(std::make_unique<FixedVertexGroupType>(id, vertices, new_fixedValue));
              } else if (new_vertex_type_index == 1) { // Force
                  ForceDirection dir = static_cast<ForceDirection>(new_froceDirection_index);
                  groups.push_back(std::make_unique<ForceVertexGroupType>(id, vertices, new_forceValue, dir));
              }
              ctx->ClearSelectedVertices();
              new_vertex_label++;
            }
        }
    }

    // Display existing vertex groups
    ImGui::Separator();
    ImGui::Text("Vertex Groups:");
    if(ImGui::BeginChild("VertexGroupList", ImVec2(0, 200),true)){
      for (auto it = groups.begin(); it != groups.end(); ) {
          const auto& group = *it;
          std::string labelTypeStr = VertexGroupLabelTypeStrings[static_cast<int>(group->getLabelType())];
          std::string detailsStr = "Label ID: " + std::to_string(group->getLabelID()) + ", Type: " + labelTypeStr + ", Num Vertices: " + std::to_string(group->getPoints().size());
          if (group->getLabelType() == VertexGroupLabelType::Force) {
              const auto* forceGroup = dynamic_cast<const ForceVertexGroupType*>(group.get());
              if (forceGroup) {
                  detailsStr += ", Force: " + forceGroup->generateRhsPart();
              }
          }
          ImGui::Text("%s", detailsStr.c_str());
          ImGui::SameLine();

          std::string removeButtonLabel = "Remove##" + std::to_string(group->getLabelID());
          if (ImGui::Button(removeButtonLabel.c_str())) {
              it = groups.erase(it);
          } else {
              ++it;
          }
      }
    }
    ImGui::EndChild();
    ImGui::BeginDisabled(groups.empty() || project->HasTetrahedralMeshGenerated() == false);
    if (ImGui::Button("Label Apply to Mesh")){
        if (groups.empty()) {
            ImGui::OpenPopup("NoGroupsPopup");
        } else {
            if (project->HasTetrahedralMeshGenerated() != false) {
                project->ApplyLabel(std::move(groups));
                groups.clear();
            } else {
                ImGui::OpenPopup("NoMeshPopup");
            }
        }
    }
    ImGui::EndDisabled();

    if (ImGui::BeginPopup("SameLabelPopup")) {
        ImGui::Text("Same label index already exists. Please choose a different index.");
        
        if (ImGui::Button("Close")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("NoGroupsPopup")) {
        ImGui::Text("No vertex groups to label. Please add at least one vertex group before labeling the mesh.");
        
        if (ImGui::Button("Close")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("NoMeshPopup")) {
        ImGui::Text("No mesh generated. Please generate a mesh before labeling.");
        
        if (ImGui::Button("Close")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }


    ImGui::End();
}

LabelUI::LabelUI(RootUICtx* rootUICtx) : UI(rootUICtx){}