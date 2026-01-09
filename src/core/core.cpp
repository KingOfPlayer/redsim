
#include "core.h"

Core::Core(){
    window = new Window(800, 600, "Main Window");
}

void Core::Run(){
    while (!window->isWindowShouldClose())
    {
        window->update();
    }
}

Core::~Core(){
    delete window;
}