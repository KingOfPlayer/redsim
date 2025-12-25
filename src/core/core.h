//#include "WindowManager.h"
//#include "renderer.h"

#include "Window.h"
class Core{
Window* window;

public:
    Core(){
        window = new Window(800, 600, "Main Window");
    }

    void Run(){
        while (!window->isWindowShouldClose())
        {
            window->update();
        }
    }

    ~Core(){
        delete window;
    }
};