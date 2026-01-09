#include "ui.h"

UI::UI(RootUICtx* rootUICtx) {
    this->rootUI = rootUICtx;
}
RootUICtx* UI::GetRootUIContext(){
    return rootUI;
};