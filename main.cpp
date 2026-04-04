#include <iostream>
#include <ostream>
#include <glad/glad.h> //glad should always be put first to prevent redefinition use of OpenGL
#include <GLFW/glfw3.h>

//glfw will autoatically fill in data
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    //set viewport sie for opengl you can make this smaller than the window size and render stuff behind it
    glViewport(0, 0, width, height);
}

int main() {
    glfwInit(); // Initiates glfw (returns GL_TRUE if successfull)

    //configure the next glfwCreateWindow() with glfwWindowHint();
    //tells glfw what OpenGL Version will be used Program will crash if client does not have proper version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); //Mac only

    //create window object
    //define how window should be set up and creates it
    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    //makes a context for the window and assigns to it
    glfwMakeContextCurrent(window);

    //initialize GLAD before we can use any opengl functions
    //cast's the glfw function which gives the OS specific function for GLAD to find
    //the OpenGL function pointer (memory address of opengl executable command)(hardware specific)
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    //tell glfw to call function when window resize; glfw will autoatically fill in data for framebuffer_size_callback
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    //render loop
    while (!glfwWindowShouldClose(window)) {
        glfwSwapBuffers(window); //swaps color buffer in window
        glfwPollEvents();//checks for events in window(keys, mouse, etc)
    }

    //clean glfw resources;
    glfwTerminate();
    return 0;
}