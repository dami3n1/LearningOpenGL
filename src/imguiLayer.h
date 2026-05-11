#ifndef IMGUILAYER_H
#define IMGUILAYER_H

#pragma once

class imguiLayer
{
public:
    static bool imguiSetup(GLFWwindow *window);
    static void imguiRender();
    static void customWindow1(float &mixValue);
    static void Shutdown();

private:

};

#endif