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

void imguiLayer::customWindow1(float &mixValue, float &fov, bool &customRatio, float &aspectRatioX, float &aspectRatioY, float &x, float &y, float &z)
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