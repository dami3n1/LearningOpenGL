//
// Created by user on 5/26/26.
//
#include "camera.h"
#include "inputHandler.h"
#include <imgui.h>
#include "global.h"
#include <algorithm>
#include "logger.h"

void inputHandler::setupMouse(GLFWwindow *window)
{
    glfwSetScrollCallback(window, scroll_callback);

    glfwSetCursorPosCallback(window, mouse_callback);
}

void inputHandler::scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    globalApplication::camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void inputHandler::mouse_callback(GLFWwindow *window, double xposIn, double yposIn)
{
    if (globalApplication::input.imguitoggle)
        return;

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (globalApplication::input.firstMouse)
    {
        globalApplication::input.lastX = xpos;
        globalApplication::input.lastY = ypos;
        globalApplication::input.firstMouse = false;
    }

    float xoffset = xpos - globalApplication::input.lastX;
    float yoffset = globalApplication::input.lastY - ypos; // reversed since y-coordinates go from bottom to top

    globalApplication::input.lastX = xpos;
    globalApplication::input.lastY = ypos;

    globalApplication::camera.ProcessMouseMovement(xoffset, yoffset);
}

void inputHandler::processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    int tabState = glfwGetKey(window, GLFW_KEY_R);

    if (tabState == GLFW_PRESS && !globalApplication::input.tabwasPressed)
    {
        globalApplication::input.tabwasPressed = true;

        globalApplication::input.imguitoggle = !globalApplication::input.imguitoggle;

        ImGuiIO &io = ImGui::GetIO();

        if (globalApplication::input.imguitoggle)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

            io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
            io.ConfigFlags &= ~ImGuiConfigFlags_NoKeyboard;
        }
        else
        {
            io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
            io.ConfigFlags |= ImGuiConfigFlags_NoKeyboard;

            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

            globalApplication::input.firstMouse = true;
        }
    }
    else if (tabState == GLFW_RELEASE)
    {
        globalApplication::input.tabwasPressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        globalApplication::camera.ProcessKeyboard(FORWARD, globalApplication::input.deltaTime, 0, 0);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        globalApplication::camera.ProcessKeyboard(BACKWARD, globalApplication::input.deltaTime, 0, 0);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        globalApplication::camera.ProcessKeyboard(LEFT, globalApplication::input.deltaTime, 0, 0);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        globalApplication::camera.ProcessKeyboard(RIGHT, globalApplication::input.deltaTime, 0, 0);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        globalApplication::camera.ProcessKeyboard(UP, globalApplication::input.deltaTime, 0, 0);
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        globalApplication::camera.ProcessKeyboard(DOWN, globalApplication::input.deltaTime, 0, 0);
}
