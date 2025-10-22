#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

// --- Shaders ---
const char* vertexShaderSource = R"glsl(
    #version 330 core
    layout(location = 0) in vec3 aPos;
    layout(location = 1) in vec3 aColor;
    out vec3 ourColor;
    void main() {
        gl_Position = vec4(aPos, 1.0);
        ourColor = aColor;
    }
)glsl";

const char* fragmentShaderSource = R"glsl(
    #version 330 core
    in vec3 ourColor;
    out vec4 FragColor;
    void main() {
        FragColor = vec4(ourColor, 1.0);
    }
)glsl";

int main() {
    // --- Initialize GLFW ---
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Rainbow Triangle + ImGui", nullptr, nullptr);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // --- Initialize GLAD ---
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

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
    float vertices[] = {
         0.0f,  0.5f, 0.0f,  1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,
         0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f
    };

    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenRenderbuffers(1, &myFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, myFBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, fboTextureWidth, fboTextureHeight);
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

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking
                                      | ImGuiWindowFlags_NoTitleBar
                                      | ImGuiWindowFlags_NoCollapse
                                      | ImGuiWindowFlags_NoResize
                                      | ImGuiWindowFlags_NoMove
                                      | ImGuiWindowFlags_NoBringToFrontOnFocus
                                      | ImGuiWindowFlags_NoNavFocus;

        ImGui::Begin("DockSpace Demo", nullptr, window_flags);
        ImGui::PopStyleVar(2);

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
            //glBindTexture(GL_TEXTURE_2D, 0);
            // 1. Bind your FBO
        glBindFramebuffer(GL_FRAMEBUFFER, myFBO);

        // Set viewport to match FBO size
        //glViewport(0, 0, fboTextureWidth, fboTextureHeight);

        // Clear and render your scene
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // Back to default framebuffer (screen)
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
            
        }
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
