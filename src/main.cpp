#include <glad/glad.h>
#include <GLFW/glfw3.h>
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

// --- Shaders ---
const char* vertexShaderSource = R"glsl(
    #version 330 core
    layout(location = 0) in vec2 aPos;
    layout(location = 1) in float aHeat;

    out float vHeat;

    void main()
    {
        vHeat = aHeat;
        gl_Position = vec4(aPos.xy, 0.0, 1.0);
    }
)glsl";

const char* fragmentShaderSource = R"glsl(
    #version 330 core
    in float vHeat;
    uniform vec3 color; // for edges, if used
    uniform bool isLine;
    out vec4 FragColor;

    vec3 thermal_colormap(float t)
    {
        t = clamp(t, 0.0, 1.0);
        if (t < 0.25) return mix(vec3(0.0,0.0,0.3), vec3(0.0,0.2,1.0), t/0.25);
        if (t < 0.5)  return mix(vec3(0.0,0.2,1.0), vec3(0.0,1.0,1.0), (t-0.25)/0.25);
        if (t < 0.75) return mix(vec3(0.0,1.0,1.0), vec3(1.0,1.0,0.0), (t-0.5)/0.25);
        return mix(vec3(1.0,1.0,0.0), vec3(1.0,0.0,0.0), (t-0.75)/0.25);
    }

    void main()
    {
        if (isLine)
            FragColor = vec4(color, 1.0);
        else
            FragColor = vec4(thermal_colormap(vHeat), 1.0);
    }
)glsl";


struct Vertex {
    float x, y;
    float heat;
};

void GenerateHeatGrid(int cols, int rows, std::vector<Vertex>& vertices, std::vector<unsigned int>& indices)
{
    vertices.clear();
    indices.clear();
    float margin = 0.2f;
    float x0 = -1.0f + margin;      // left edge
    float y0 = -1.0f + margin;      // bottom edge
    float x1 =  1.0f - margin;      // right edge
    float y1 =  1.0f - margin;      // top edge

    float dx = (x1 - x0) / cols;
    float dy = (y1 - y0) / rows;

    // ---- Generate vertices ----
    for (int j = 0; j <= rows; ++j)
    {
        for (int i = 0; i <= cols; ++i)
        {
            float x = x0 + i * dx;
            float y = y0 + j * dy;

            // Simple heat pattern: gradient across grid
            // y=3*(x*x)-2*(x*x*x);
            // y=20x^{7}-70x^{6}+84x^{5}-35x^{4}
            float heat = (float)i / cols;
            heat = 20.0f * heat * heat * heat * heat * heat * heat * heat
                 - 70.0f * heat * heat * heat * heat * heat * heat
                 + 84.0f * heat * heat * heat * heat * heat
                 - 35.0f * heat * heat * heat * heat;
                 heat *= -1.0f;

            vertices.push_back({ x, y, heat });
        }
    }

    // ---- Generate indices ----
    for (int j = 0; j < rows; ++j)
    {
        for (int i = 0; i < cols; ++i)
        {
            int row1 = j * (cols + 1);
            int row2 = (j + 1) * (cols + 1);

            unsigned int v0 = row1 + i;
            unsigned int v1 = row1 + i + 1;
            unsigned int v2 = row2 + i;
            unsigned int v3 = row2 + i + 1;

            // Two triangles per cell
            indices.push_back(v0); indices.push_back(v1); indices.push_back(v3);
            indices.push_back(v0); indices.push_back(v3); indices.push_back(v2);
        }
    }
}

std::vector<unsigned int> generateEdges(const std::vector<unsigned int>& indices)
{
    std::set<std::pair<unsigned int, unsigned int>> uniqueEdges;
    std::vector<unsigned int> edges;

    for (size_t i = 0; i < indices.size(); i += 3)
    {
        unsigned int i0 = indices[i];
        unsigned int i1 = indices[i + 1];
        unsigned int i2 = indices[i + 2];

        unsigned int triEdges[3][2] = {
            {i0, i1},
            {i1, i2},
            {i2, i0}
        };

        for (auto& e : triEdges)
        {
            unsigned int a = std::min(e[0], e[1]);
            unsigned int b = std::max(e[0], e[1]);
            uniqueEdges.insert({a, b});
        }
    }

    // Flatten to edge index list
    for (auto& e : uniqueEdges)
    {
        edges.push_back(e.first);
        edges.push_back(e.second);
    }

    return edges;
}

void FileOpenDialog(){
    FilePath file = FileModule::SelectFile();
    GCodeModule gcode;
    
    gcode.OpenFile(&file);
    printf("File path length: %d\n", file.path_length);
}

int main() {
    // --- Initialize GLFW ---
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor *monitor = glfwGetPrimaryMonitor();

    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

    
    // Make the window borderless
    //glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Rainbow Triangle + ImGui", nullptr, nullptr);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    glfwSetWindowPos(window, 100, 100);

    // --- Initialize GLAD ---
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    // --- Setup ImGui ---
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // --- Create triangle data ---
    // [x, y, u, v, heat]
    /*float vertices[] = {
        // Rectangle 1 (left, cooler region)
        -0.9f, -0.5f, 0.1f,   // 0 bottom-left
        -0.5f, -0.5f, 0.3f,   // 1 bottom-mid
        -0.1f, -0.5f, 0.2f,   // 2 bottom-right
        -0.9f,  0.5f, 0.4f,   // 3 top-left
        -0.5f,  0.5f, 0.6f,   // 4 top-mid
        -0.1f,  0.5f, 0.5f,   // 5 top-right

        // Rectangle 2 (right, hotter region)
        0.1f, -0.5f, 0.5f,   // 6 bottom-left
        0.5f, -0.5f, 0.8f,   // 7 bottom-mid
        0.9f, -0.5f, 1.0f,   // 8 bottom-right
        0.1f,  0.5f, 0.7f,   // 9 top-left
        0.5f,  0.5f, 0.9f,   // 10 top-mid
        0.9f,  0.5f, 0.95f   // 11 top-right
    };


    unsigned int indices[] = {
        // Rectangle 1
        0, 1, 4,  0, 4, 3,   // left cell
        1, 2, 5,  1, 5, 4,   // right cell

        // Rectangle 2
        6, 7, 10,  6, 10, 9,  // left cell
        7, 8, 11,  7, 11, 10  // right cell
    };*/
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    GenerateHeatGrid(4, 3, vertices, indices);

    std::vector<unsigned int> edges = generateEdges(indices);

    unsigned int VAO, VBO, EBO, edgeEBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenBuffers(1, &edgeEBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, edgeEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, edges.size() * sizeof(unsigned int), edges.data(), GL_STATIC_DRAW);

    GLsizei stride = 3 * sizeof(float);
    glEnableVertexAttribArray(0); // position
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(1); // heat
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, stride, (void*)(2 * sizeof(float)));

    // --- Compile shaders ---
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    GLuint myFBO;
    GLuint fboTexture;
    int fboTextureWidth = 800;
    int fboTextureHeight = 600;

    // 1️⃣ Generate framebuffer
    glGenFramebuffers(1, &myFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, myFBO);

    // 2️⃣ Create texture to render into
    glGenTextures(1, &fboTexture);
    glBindTexture(GL_TEXTURE_2D, fboTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fboTextureWidth, fboTextureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenRenderbuffers(1, &myFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, myFBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1920, 1080);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, myFBO);

    // 3️⃣ Attach texture to framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTexture, 0);

    // 4️⃣ Check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        printf("ERROR: Framebuffer is not complete!\n");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // --- Main loop ---
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking
                                      | ImGuiWindowFlags_NoTitleBar
                                      | ImGuiWindowFlags_NoCollapse
                                      | ImGuiWindowFlags_NoResize
                                      | ImGuiWindowFlags_NoMove
                                      | ImGuiWindowFlags_NoBringToFrontOnFocus
                                      | ImGuiWindowFlags_NoNavFocus;

        ImGui::Begin("DockSpace Demo", nullptr, window_flags);
        ImGui::PopStyleVar(3);

        // Create the dockspace
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
        ImGui::End();

        // --- UI ---
        ImGui::Begin("Control Panel");
        ImGui::Text("Hello from ImGui!");
        ImGui::Text("Use this panel to adjust parameters later.");
        ImGui::End();

        ImGui::Begin("Button Window");
        if (ImGui::Button("Click me")) {
            printf("Button clicked!\n");
            std::thread(FileOpenDialog).detach();
        }

        ImGui::End();


        


        // Draw FBO texture inside ImGui window
        ImGui::Begin("Viewport");
        ImVec2 size = ImGui::GetContentRegionAvail();

        ImGui::Image((void*)(intptr_t)fboTexture, size,
            ImVec2(0, 1),  // top-left corner of the texture
            ImVec2(1, 0)   // bottom-right corner of the texture
        );

        if (size.x != fboTextureWidth || size.y != fboTextureHeight) {
            fboTextureWidth = size.x;
            fboTextureHeight = size.y;

            glBindTexture(GL_TEXTURE_2D, fboTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fboTextureWidth, fboTextureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
            glBindTexture(GL_TEXTURE_2D, 0);

            glBindRenderbuffer(GL_RENDERBUFFER, myFBO);
	        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, fboTextureWidth, fboTextureHeight);
            glBindRenderbuffer(GL_RENDERBUFFER, 0);

            // 1. Bind your FBO
        }

        glBindFramebuffer(GL_FRAMEBUFFER, myFBO);
        
        // Set viewport to match FBO size
        //glViewport(0, 0, fboTextureWidth, fboTextureHeight);

        // Clear and render your scene
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glUniform1i(glGetUniformLocation(shaderProgram, "isLine"), GL_FALSE);
        glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);
        
        glLineWidth(1.0f);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, edgeEBO);
        glUniform1i(glGetUniformLocation(shaderProgram, "isLine"), GL_TRUE);
        glUniform3f(glGetUniformLocation(shaderProgram, "color"), 1.0f, 1.0f, 1.0f);
        glDrawElements(GL_LINES, (GLsizei)edges.size(), GL_UNSIGNED_INT, 0);


        // Back to default framebuffer (screen)
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Back to default framebuffer (screen)
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        ImGui::End();

        // --- Draw ImGui ---
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        glfwSwapBuffers(window);
    }

    // --- Cleanup ---
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glfwTerminate();
    return 0;
}
