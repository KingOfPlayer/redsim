
#pragma once
#include <vector>
#include <imgui.h>

#include "ui.h"


class ModelGenUI : public UI{
    float nozzleDiameter = 0.60f;
    int qualityIndex = 0;
    bool nef_based = false;
    bool remesh_after_layers = false;
    float remesh_target_length = 1.1f;
    float remesh_edge_angle = 45.0f;
    int remesh_iterations = 1;


    float tetrahedral_cell_size        = 2.0;
    float tetrahedral_cell_radius_edge = 2.0;
    float tetrahedral_facet_angle      = 25.0;
    float tetrahedral_facet_size       = 2.0;
    float tetrahedral_facet_distance   = 0.5;
	int    tetrahedral_remesh_iterations = 1;  

    void render() override;
public:
    ModelGenUI(RootUICtx* rootUICtx);
};