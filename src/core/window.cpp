#include "window.h"

Window::Window(int width, int height, const char* title){
    if (!glfwInit()) {
        window = nullptr;
        return;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        window = nullptr;
        return;
    }
    glfwMakeContextCurrent(window);

    glfwSetWindowPos(window, 100, 100);

    // --- Initialize GLAD ---
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        assert(false);
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    printf("Window and OpenGL context initialized successfully.\n");

    project = new Project();
    printf("Project created successfully.\n");
    rootUICtx = new RootUICtx(project);
    printf("RootUICtx created successfully.\n");
    rootUI = new RootUI(window,rootUICtx);
    printf("RootUI created successfully.\n");
}

void Window::update(){
    glfwPollEvents();

    rootUI->render();

    glfwSwapBuffers(window);
}

Window::~Window(){
    if(window){
        glfwDestroyWindow(window);
        glfwTerminate();
    }
}

bool Window::isWindowShouldClose(){
    return glfwWindowShouldClose(window);
}