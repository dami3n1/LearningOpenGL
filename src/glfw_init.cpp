#include "glfw_init.h"
#include <glad/glad.h> //glad should always be put first to prevent redefinition use of OpenGL
#include <GLFW/glfw3.h>
#include <iostream>

glfw_init::glfw_init()
{
    if (!glfwInit()) // Initiates glfw (returns GL_TRUE if successfull)
    {
        std::cout << "Failed to initialize GLFW" << std::endl;
        return;
    }

    // configure the next glfwCreateWindow() with glfwWindowHint();
    // tells glfw what OpenGL Version will be used Program will crash if client does not have proper version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
}

glfw_init::~glfw_init()
{
    // clean glfw resources;
    glfwTerminate();
}