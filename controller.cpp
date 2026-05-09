#include "controller.h"

Controller::Controller(int joystickID)
{
    jid = joystickID;
}

void Controller::update()
{
    if (!glfwJoystickPresent(jid))
    {
        buttons = nullptr;
        axes = nullptr;
        return;
    }

    axes = glfwGetJoystickAxes(jid, &axisCount);
    buttons = glfwGetJoystickButtons(jid, &buttonCount);
}

bool Controller::isConnected() const
{
    return glfwJoystickPresent(jid);
}

bool Controller::button(int i) const
{
    return buttons && i < buttonCount && buttons[i] == GLFW_PRESS;
}

float Controller::axis(int i) const
{
    return (axes && i < axisCount) ? axes[i] : 0.0f;
}

// ================= BUTTON MAPPING =================
// Standard Xbox layout (most GLFW + XInput controllers)

bool Controller::A() const { return button(0); }
bool Controller::B() const { return button(1); }
bool Controller::X() const { return button(2); }
bool Controller::Y() const { return button(3); }

bool Controller::LB() const { return button(4); }
bool Controller::RB() const { return button(5); }

bool Controller::Start() const { return button(7); }

// ================= STICKS =================
// Left stick: 0,1
// Right stick: 2,3
// Triggers: 4,5 (usually)

Vec2 Controller::leftStick() const
{
    return { axis(0), axis(1) };
}

Vec2 Controller::rightStick() const
{
    return { axis(2), axis(3) };
}

float Controller::leftTrigger() const { return axis(4); }
float Controller::rightTrigger() const { return axis(5); }