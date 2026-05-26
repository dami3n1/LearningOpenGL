//
// Created by user on 5/26/26.
//
#include "GLFW/glfw3.h"
#ifndef MYPROJECT_INPUT_HANDLER_H
#define MYPROJECT_INPUT_HANDLER_H

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow *window, double xposIn, double yposIn);

class inputHandler {
public:
    bool imguitoggle = false;
    bool tabwasPressed = false;
    bool firstMouse = true;

    float lastX = 800 / 2.0f;
    float lastY = 600 / 2.0f;

    // timing
    float deltaTime = 0.0f; // time between current frame and last frame
    float lastFrame = 0.0f;


    static void processInput(GLFWwindow *window);
};


#endif //MYPROJECT_INPUT_HANDLER_H
