#include "controller.h"
#include "logger.h"
#include "global.h"
#include <algorithm>

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

void Controller::showControllers()
{
    // Detect connected controllers
    for (int jid = GLFW_JOYSTICK_1; jid <= GLFW_JOYSTICK_LAST; jid++)
    {
        if (glfwJoystickPresent(jid))
        {
            logger(INFO, "Connected: " + std::string(glfwGetJoystickName(jid)));

            if (glfwJoystickIsGamepad(jid))
            {
                logger(INFO, "Recognized as gamepad.");
            }
        }
    }
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
    return {axis(0), axis(1)};
}

Vec2 Controller::rightStick() const
{
    return {axis(3), axis(4)};
}

Vec2 Controller::processJoystickInput(Vec2 Joystick)
{
    Vec2 JoystickOut{0.0f, 0.0f};

    if (fabs(Joystick.x) >= globalApplication::controller.deadzone)
        JoystickOut.x = Joystick.x * globalApplication::controller.controlCamSpeed * globalApplication::input.deltaTime;

    if (fabs(Joystick.y) >= globalApplication::controller.deadzone)
       JoystickOut.y = (-Joystick.y) * globalApplication::controller.controlCamSpeed * globalApplication::input.deltaTime;

    return JoystickOut;
}

void Controller::processController()
{
    // movement control
    Vec2 leftJoystick = globalApplication::controller.processJoystickInput(globalApplication::controller.leftStick());

    globalApplication::camera.ProcessKeyboard(CONTROLLER, globalApplication::input.deltaTime * globalApplication::controller.controlMovSpeed, leftJoystick.x, leftJoystick.y);

    // camera control
    Vec2 rightJoystick = globalApplication::controller.processJoystickInput(globalApplication::controller.rightStick());

    globalApplication::camera.ProcessMouseMovement(rightJoystick.x, rightJoystick.y);

    if (globalApplication::controller.A())
        globalApplication::camera.ProcessKeyboard(UP, globalApplication::input.deltaTime, 0, 0);
    if (globalApplication::controller.B())
        globalApplication::camera.ProcessKeyboard(DOWN, globalApplication::input.deltaTime, 0, 0);
    if (globalApplication::controller.Y())
        glfwSetWindowShouldClose(globalApplication::window, true);

    globalApplication::camera.Zoom = std::clamp(globalApplication::camera.Zoom, 1.0f, 170.0f);

    if (rightTrigger() >= 0.1f)
    {
        globalApplication::camera.Zoom -= 0.3;
    }
    if (leftTrigger() >= 0.1f)
    {
        globalApplication::camera.Zoom += 0.3;
    }
}

float Controller::leftTrigger() const { return axis(2); }
float Controller::rightTrigger() const { return axis(5); }