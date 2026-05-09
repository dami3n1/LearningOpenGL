#include <glad/glad.h> //glad should always be put first to prevent redefinition use of OpenGL
#include <GLFW/glfw3.h>
#include "windowSystem.h"
#include "logger.h"

// glfw will autoatically fill in data
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    // set viewport sie for opengl you can make this smaller than the window size and render stuff behind it
    glViewport(0, 0, width, height);
}

bool windowSystem::glfw_init()
{
    if (!glfwInit()) // Initiates glfw (returns GL_TRUE if successfull)
    {
        logger(ERROR, "Failed to initialize GLFW");\
        return false;
    }

    // configure the next glfwCreateWindow() with glfwWindowHint();
    // tells glfw what OpenGL Version will be used Program will crash if client does not have proper version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif


    return true;
}

GLFWwindow* windowSystem::makeWindow(int screenWidth, int screenHeight, const char* title)
{
    // create window object
    // define how window should be set up and creates it
    GLFWwindow *window = glfwCreateWindow(screenWidth, screenHeight, title, NULL, NULL);
    if (window == NULL)
    {
        logger(ERROR, "Failed to create GLFW window");
        glfwTerminate();
        return nullptr;
    }
    // makes a context for the window and assigns to it
    glfwMakeContextCurrent(window);
    // tell glfw to call function when window resize; glfw will autoatically fill in data for framebuffer_size_callback
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    return window;
}

void windowSystem::glfw_shutdown()
{
    logger(WARN, "Terminating GLFW");
    // clean glfw resources;
    glfwTerminate();
}