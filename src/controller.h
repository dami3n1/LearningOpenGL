#pragma once
#include <GLFW/glfw3.h>

struct Vec2
{
    float x, y;
};

class Controller
{
public:
    Controller(int joystickID = GLFW_JOYSTICK_1);

    void update();
    bool isConnected() const;
    void showControllers();

    // Buttons (Xbox layout)
    bool A() const;
    bool B() const;
    bool X() const;
    bool Y() const;

    bool LB() const;
    bool RB() const;

    bool Start() const;

    // Sticks
    Vec2 leftStick() const;
    Vec2 rightStick() const;

    float leftTrigger() const;
    float rightTrigger() const;

private:
    int jid;

    const unsigned char* buttons = nullptr;
    const float* axes = nullptr;

    int buttonCount = 0;
    int axisCount = 0;

    bool button(int i) const;
    float axis(int i) const;
};