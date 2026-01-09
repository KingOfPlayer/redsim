#include "rootuictx.h"

RootUICtx::~RootUICtx() {
    delete project;
}
Project* RootUICtx::getProject() {
    return project;
}