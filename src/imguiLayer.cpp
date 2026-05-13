#include <glad/glad.h> //glad should always be put first to prevent redefinition use of OpenGL
#include <GLFW/glfw3.h>
#include "imguiLayer.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

bool imguiLayer::imguiSetup(GLFWwindow *window)
{

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true); // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
    ImGui_ImplOpenGL3_Init();

    return true;
}

void imguiLayer::imguiRender()
{
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
}

void imguiLayer::customWindow1(float &mixValue, float &fov, bool &customRatio, float &aspectRatioX, float &aspectRatioY, float &x, float &y, float &z,float &directionX, float &directionY, float &directionZ, float deltaTime)
{
    ImGui::NewFrame();
    ImGui::Begin("My Window");

    ImGui::SliderFloat("Texture Mix", &mixValue, 0.0f, 1.0f);
    ImGui::SliderFloat("FoV", &fov, 0.0f, 180.0f);
    ImGui::Checkbox("Custom Ratio", &customRatio);

    if (customRatio)
    {
        ImGui::SliderFloat("Aspect Ratio X", &aspectRatioX, 0.0f, 1000.0f);
        ImGui::SliderFloat("Aspect Ratio Y", &aspectRatioY, 0.0f, 1000.0f);
    }
    ImGui::SliderFloat("View X", &x, -10.0f, 10.0f);
    ImGui::SliderFloat("View Y", &y, -10.0f, 10.0f);
    ImGui::SliderFloat("View Z", &z, -10.0f, 10.0f);
    ImGui::SliderFloat("Direction X", &directionX, -1.0f, 1.0f);
    ImGui::SliderFloat("Direction Y", &directionY, -1.0f, 1.0f);
    ImGui::SliderFloat("Direction Z", &directionZ, -1.0f, 1.0f);

    static float fpsHistory[120] = {};
    static float avgHistory[120] = {};
    static int index = 0;

    float fps = (deltaTime > 0.0f) ? (1.0f / deltaTime) : 0.0f;

    static float fpsAvg = 0.0f;
    fpsAvg = fpsAvg * 0.9f + fps * 0.1f;

    // store values
    fpsHistory[index] = fps;
    avgHistory[index] = fpsAvg;

    index = (index + 1) % 120;

    // text
    ImGui::Text("FPS: %.1f", fps);
    ImGui::Text("Avg FPS: %.1f", fpsAvg);
    ImGui::Text("ImGui FPS: %.1f", ImGui::GetIO().Framerate);

    // graphs
    ImGui::PlotLines("FPS (Instant)", fpsHistory, 120, index, nullptr, 0.0f, 200.0f, ImVec2(0, 80));
    ImGui::PlotLines("FPS (Average)", avgHistory, 120, index, nullptr, 0.0f, 200.0f, ImVec2(0, 80));

    

    ImGui::End();
    // Rendering
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void imguiLayer::Shutdown()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}