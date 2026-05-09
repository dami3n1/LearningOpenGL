#include <glad/glad.h> //glad should always be put first to prevent redefinition use of OpenGL
#include <GLFW/glfw3.h>
#include "glfw_init.h"
#include "logger.h"

// glfw will autoatically fill in data
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    // set viewport sie for opengl you can make this smaller than the window size and render stuff behind it
    glViewport(0, 0, width, height);
}

glfw_init::glfw_init()
{
    if (!glfwInit()) // Initiates glfw (returns GL_TRUE if successfull)
    {
        logger(ERROR, "Failed to initialize GLFW");
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

GLFWwindow* glfw_init::makeWindow(int screenWidth, int screenHeight, const char* title)
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

    // initialize GLAD before we can use any opengl functions
    // cast's the glfw function which gives the OS specific function for GLAD to find
    // the OpenGL function pointer (memory address of opengl executable command)(hardware specific)
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        logger(ERROR, "Failed to initialize GLAD");
    }

    return window;
}

glfw_init::~glfw_init()
{
    logger(WARN, "Terminating GLFW");
    // clean glfw resources;
    glfwTerminate();
}