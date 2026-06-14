
#pragma once
#include <vector>
#include <imgui.h>

#include "ui.h"

#include "../../modules/freefem/freefemtype.h"


class LabelUI : public UI{
    std::vector<std::unique_ptr<VertexGroupBaseType>> groups;

    int new_vertex_label = 1;
    int new_vertex_type_index = 0;

    // Fixed
	glm::vec3 new_fixedValue;

    // Force
    int new_froceDirection_index = 0;
    int new_forceValue;

    void render() override;
public:
    LabelUI(RootUICtx* rootUICtx);
};