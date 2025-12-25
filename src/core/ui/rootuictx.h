#pragma once
#include "../../modules/project/project.h"

class RootUICtx {
    Project* project;
public:
    RootUICtx(Project* proj) : project(proj) {};
    ~RootUICtx() {
        delete project;
    }
    Project* getProject() {
        return project;
    }
};