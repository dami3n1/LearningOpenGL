//
// Created by user on 5/26/26.
//

#ifndef MYPROJECT_GLOBAL_H
#define MYPROJECT_GLOBAL_H
#include "camera.h"
#include "inputHandler.h"
#include "controller.h"

namespace globalApplication {
    extern inputHandler input;
    extern Camera camera;
    extern Controller controller;
    extern GLFWwindow *window;
};

#endif //MYPROJECT_GLOBAL_H
