#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>
#include <set>
#include <utility>
#include <vector>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "./modules/file/file.h"
#include "./modules/gcode/gcode.h"
#include <thread>
#include "core/core.h"

int main() {
    Core core;
    core.Run();
    return 0;
}
