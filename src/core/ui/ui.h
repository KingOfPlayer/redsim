#pragma once
#include "rootuictx.h"

class UI{
RootUICtx* rootUI;
public:
    virtual void render() = 0;
    UI(RootUICtx* rootUICtx);
    RootUICtx* GetRootUIContext();
};