#include "../../modules/project/project.h"
#pragma once

class RootUICtx {
    Project* project;
public:
    RootUICtx(Project* proj) : project(proj){};
    ~RootUICtx();
    Project* getProject();
};