#ifndef GLFW_INIT_H
#define GLFW_INIT_H

#pragma once

class glfw_init
{
public:
    glfw_init();
    GLFWwindow* makeWindow(int screenWidth, int screenHeight, const char* title);
    ~glfw_init();

private:

};

#endif