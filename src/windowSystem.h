#ifndef WINDOW_SYSTEM_H
#define WINDOW_SYSTEM_H

#pragma once

class windowSystem
{
public:
    static bool glfw_init();
    static GLFWwindow* makeWindow(int screenWidth, int screenHeight, const char* title);
    static void glfw_shutdown();

private:

};

#endif