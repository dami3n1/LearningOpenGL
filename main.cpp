#include <iostream>
#include <ostream>
#include <glad/glad.h> //glad should always be put first to prevent redefinition use of OpenGL
#include <GLFW/glfw3.h>

//glfw will autoatically fill in data
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    //set viewport sie for opengl you can make this smaller than the window size and render stuff behind it
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)// if it's not pressed, glfwGetKey returns GLFW_RELEASE
        glfwSetWindowShouldClose(window, true);
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
        //input function called each frame
        processInput(window);

        //glClear uses this color to clear the screen (sets it doesnt have to be called always)
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        //clear the buffer so you wont see previous frame (you have to specify which one)
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(window); //swaps color buffer in window
        glfwPollEvents();//processes events received in window and returns a response(if requested)
    }

    //clean glfw resources;
    glfwTerminate();
    return 0;
}