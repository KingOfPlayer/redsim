#include "rootuictx.h"

#include "../../modules/project/project.h"

RootUICtx::~RootUICtx() {
    delete project;
}
Project* RootUICtx::getProject() {
    return project;
}