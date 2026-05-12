#ifndef IMGUILAYER_H
#define IMGUILAYER_H

#pragma once

class imguiLayer
{
public:
    static bool imguiSetup(GLFWwindow *window);
    static void imguiRender();
    static void customWindow1(float &mixValue, float &fov, float &aspectRatioX, float &aspectRatioY, float &x, float &y, float &z);
    static void Shutdown();

private:

};

#endif