#include "labelui.h"

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

    ImGui::End();
}

LabelUI::LabelUI(RootUICtx* rootUICtx) : UI(rootUICtx){}