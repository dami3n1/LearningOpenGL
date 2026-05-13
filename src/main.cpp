#include <cmath>
#include <iostream>
#include <iomanip>
#include <ostream>

#include "shader_reader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../ext/stb/stb_image.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "controller.h"
#include <algorithm>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "windowSystem.h"
#include "logger.h"
#include "imguiLayer.h"

const unsigned int SCREEN_HEIGHT = 600;
const unsigned int SCREEN_WIDTH = 800;

// stores how much we're seeing of either texture
float mixValue = 0.2f;
Controller controller;
float lastX = SCREEN_WIDTH / 2.0f, lastY = SCREEN_HEIGHT / 2.0f;

float yaw = -90.0f;
float pitch = 0.0f;
float fov = 45.0f;

bool firstMouse = true;
bool uiMode = true;
bool gameMode = true;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);    // this is the position of the camera in world space
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f); // this is the direction the camera is looking at (the negative z-axis)
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0);

float deltaTime = 0.0f; // Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

glm::vec3 direction;

void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    ImGuiIO &io = ImGui::GetIO();
    if (!gameMode)
        return;

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        mixValue += 0.02f; // change this value accordingly (might be too slow or too fast based on system hardware)
        if (mixValue >= 1.0f)
            mixValue = 1.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        mixValue -= 0.02f; // change this value accordingly (might be too slow or too fast based on system hardware)
        if (mixValue <= 0.0f)
            mixValue = 0.0f;
    }

    static bool tabPressed = false;

    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS)
    {
        if (!tabPressed)
        {
            gameMode = !gameMode;

            if (gameMode)
            {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }
            else
            {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

                // IMPORTANT: reset mouse so camera doesn't jump when returning
                firstMouse = true;
            }
        }
        tabPressed = true;
    }
    else
    {
        tabPressed = false;
    }

    const float cameraSpeed = 5.0f * deltaTime; // adjust accordingly
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        // cross product of cameraFront and cameraUp gives the right vector of the camera which is the direction to the right of the camera
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        // cross product of cameraFront and cameraUp gives the right vector of the camera which is the direction to the right of the camera
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

    Vec2 left = controller.leftStick();

    float speed = 1.5f;
    float deadzone = 0.2f;

    float inputY = left.y;

    if (fabs(inputY) < deadzone)
        inputY = 0.0f;

    mixValue += (-inputY) * speed * deltaTime;

    mixValue = std::clamp(mixValue, 0.0f, 1.0f);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    fov -= (float)yoffset;
    if (fov < 1.0f)
        fov = 1.0f;
    if (fov > 180.0f)
        fov = 180.0f; 
}

void texturmaker(GLuint &texture, const char *path, GLenum format)
{
    glGenTextures(1, &texture);
    // bind texture so any texture commands willl go to this texture
    glBindTexture(GL_TEXTURE_2D, texture);

    // s,t,r = x,y,z
    // texture wrapping/filtering/mipmap options
    // wrapping repeats the image horizontally and vertically
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // linearly interpolates between the two mipmaps that most closely match the size of a pixel and samples the interpolated level via nearest neighbor interpolation.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // blends colors but no mipmap option (only works when downscaling)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // use stbimage to read image data

    int width, height, nrChannels; // stb fills this with data

    unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);

    if (data)
    {
        // target: generate texture on currently bound texture (GL_TEXTURE_2D)
        // level( of detail(mipmap)): 0 = regular image
        // format of color
        // image size width and height
        // border must be 0
        // format of pixel data
        // data type of pixel data
        // pointer to the actual image data
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        // generate mipmaps
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    // free the image from memory
    stbi_image_free(data);
}

int main()
{

    if (!windowSystem::glfw_init())
    {
        logger(ERROR, "windowSystem::Failed to initialize GLFW");
        return -1; // or stop engine
    }
    GLFWwindow *window = windowSystem::makeWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "OpenGL Window");
    if (!window)
    {
        logger(ERROR, "windowSystem::Failed to create GLFW window");
        windowSystem::glfw_shutdown();
        return -1;
    }

    // disable vsync for uncapped framerate
    glfwSwapInterval(0);

    glfwSetScrollCallback(window, scroll_callback); 

    glfwSetCursorPosCallback(window, mouse_callback);

    // initialize GLAD before we can use any opengl functions
    // cast's the glfw function which gives the OS specific function for GLAD to find
    // the OpenGL function pointer (memory address of opengl executable command)(hardware specific)
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        logger(ERROR, "Failed to initialize GLAD");
        return -1;
    }

    // set glViewport so first frame renders correctly before hiting our resize function
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    if (!imguiLayer::imguiSetup(window))
    {
        logger(ERROR, "Failed to initialize ImGui");
        return -1;
    }

    Shader ourShader("../shaders/vertex.vs", "../shaders/fragment.fs");

    float vertices[] = {
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,

        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,

        -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,

        0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,

        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f};

    glm::vec3 cubePositions[] = {
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(2.0f, 5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3(2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f, 3.0f, -7.5f),
        glm::vec3(1.3f, -2.0f, -2.5f),
        glm::vec3(1.5f, 2.0f, -2.5f),
        glm::vec3(1.5f, 0.2f, -1.5f),
        glm::vec3(-1.3f, 1.0f, -1.5f)};

    // VBO = stores the actual vertex data (like positions, colors, etc.)
    // VAO = remembers how to use that data (how OpenGL should read it)
    // EBO = stores index data (which vertices to draw and in what order)

    // These variables will hold IDs (like handles) for the buffers
    unsigned int VBO, VAO, EBO;

    // Create 1 VAO and store its ID in VAO
    glGenVertexArrays(1, &VAO);

    // Create 1 VBO and store its ID in VBO
    glGenBuffers(1, &VBO);

    // Create 1 EBO and store its ID in EBO
    // glGenBuffers(1, &EBO);

    // Bind the VAO so OpenGL knows we are working with it now
    // Any setup we do will be saved inside this VAO
    glBindVertexArray(VAO);

    // Bind the VBO to GL_ARRAY_BUFFER
    // This means we are now working with this VBO for vertex data
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // Copy vertex data into the VBO
    // sizeof(vertices) = size of the data
    // vertices = the actual data
    // GL_STATIC_DRAW = data will not change much
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // GL_STREAM_DRAW: the data is set only once and used by the GPU at most a few times.
    // GL_STATIC_DRAW: the data is set only once and used many times.
    // GL_DYNAMIC_DRAW: the data is changed a lot and used many times.

    // bind EBO and copy indicies into buffer like VBO
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    // more info look at VBO
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Tell OpenGL how to read the vertex data from the VBO

    // 0 = location in the vertex shader (layout(location = 0))
    // 3 = number of values per vertex (x, y, z)
    // GL_FLOAT = type of each value
    // GL_FALSE = do NOT normalize the data
    // 3 * sizeof(float) = size of one vertex (3 floats total)
    // (void*)0 = start reading at the beginning of the data
    // IMPORTANT: this uses the VBO currently bound to GL_ARRAY_BUFFER
    // read points from vertex data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);

    // Enable the vertex attribute at location 0 so OpenGL can use it
    glEnableVertexAttribArray(0);

    // data for where to place texture
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // IMPORTANT:
    // Do NOT unbind the EBO while the VAO is still bound
    // The EBO is stored inside the VAO, so it needs to stay bound

    // Unbind the VAO so we don’t accidentally modify it later
    glBindVertexArray(0);

    // like objects textures are also refrenced by an ID
    unsigned int texture1, texture2;

    texturmaker(texture1, "../assets/container.jpg", GL_RGB);

    texturmaker(texture2, "../assets/awesomeface.png", GL_RGBA);

    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    // -------------------------------------------------------------------------------------------
    ourShader.use(); // don't forget to activate/use the shader before setting uniforms!
    ourShader.setInt("texture1", 0);
    ourShader.setInt("texture2", 1);

    controller.showControllers();

    float lastFrame = 0.0f;

    // Unbind the VBO (optional, just to avoid accidental changes later)
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    
    int width = SCREEN_WIDTH, height = SCREEN_HEIGHT;
    float aspectRatioX = width;
    float aspectRatioY = height;
    bool customaspectratio = false;

    glEnable(GL_DEPTH_TEST); // enable depth testing for 3D

    // render loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents(); // processes events received in window and returns a response(if requested)
        // input function called each frame
        processInput(window);

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if (!customaspectratio)
        {
            glfwGetFramebufferSize(window, &width, &height);
            aspectRatioX = (float)width;
            aspectRatioY = (float)height;
        }

        imguiLayer::imguiRender();
        imguiLayer::customWindow1(mixValue, fov, customaspectratio, aspectRatioX, aspectRatioY, cameraPos.x, cameraPos.y, cameraPos.z, direction.x, direction.y, direction.z, deltaTime);

        controller.update();

        // glClear uses this color to clear the screen (sets it doesnt have to be called always)
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        // clear the buffer so you wont see previous frame (you have to specify which one) (you can have multiple buffers like color, depth, stencil)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // allows to use more than one texture assigning it to the texture unit(GL_TEXTURE0)
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);

        // set the texture mix value in the shader
        ourShader.setFloat("mixValue", mixValue);

        // call shader program
        ourShader.use();

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));

        glm::vec3 direction;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

        glm::mat4 view;

        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 projection = glm::mat4(1.0f);

        projection = glm::perspective(glm::radians(fov), aspectRatioX / aspectRatioY, 0.1f, 100.0f);

        model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f)); // rotate the matrix by the current time (in radians) around the z-axis

        int viewLoc = glGetUniformLocation(ourShader.ID, "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        int projectionLoc = glGetUniformLocation(ourShader.ID, "projection");
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // call the configuration
        glBindVertexArray(VAO);
        for (unsigned int i = 0; i < 10; i++)
        {
            // calculate the model matrix for each object and pass it to shader before drawing
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            float angle = 20.0f * i;

            if (i % 3 == 0)
                angle = glfwGetTime() * 50.0f;

            model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            int modelLoc = glGetUniformLocation(ourShader.ID, "model");
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        for (int i = 0; i % 3 == 0; i++)
        {
            int modelLoc = glGetUniformLocation(ourShader.ID, "model");
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            model = glm::mat4(1.0f);
            model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));

            model = glm::translate(model, glm::vec3(-0.5f, 0.5f, 0.0f));                 // translate the matrix by (-0.5, 0.5, 0.0) (move it to the left and up)
            float scaleAmount = static_cast<float>(sin(glfwGetTime()));                  // calculate a scale factor that oscillates between 0.0 and 1.0 based on the sine of the current time
            model = glm::scale(model, glm::vec3(scaleAmount, scaleAmount, scaleAmount)); // scale the matrix by the calculated scale factor
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));            // this time take the matrix value array's first element as its memory pointer value
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        // Draw triangles using the indices stored in the currently bound EBO (GL_ELEMENT_ARRAY_BUFFER)
        // Normally, you'd have to bind the correct EBO for each object before calling glDrawElements.
        // However, VAOs remember which EBO was bound when the VAO was created.
        // So simply binding the VAO automatically binds the right EBO, making rendering easier.

        // second transformation
        // ---------------------
        int modelLoc = glGetUniformLocation(ourShader.ID, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));

        model = glm::translate(model, glm::vec3(-0.5f, 0.5f, 0.0f));                 // translate the matrix by (-0.5, 0.5, 0.0) (move it to the left and up)
        float scaleAmount = static_cast<float>(sin(glfwGetTime()));                  // calculate a scale factor that oscillates between 0.0 and 1.0 based on the sine of the current time
        model = glm::scale(model, glm::vec3(scaleAmount, scaleAmount, scaleAmount)); // scale the matrix by the calculated scale factor
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));            // this time take the matrix value array's first element as its memory pointer value
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Rendering
        // (Your code clears your framebuffer, renders your other stuff etc.)
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        // (Your code calls glfwSwapBuffers() etc.)

        glfwSwapBuffers(window); // swaps color buffer in window
    }

    imguiLayer::Shutdown();

    // de allocated resources after usage
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    windowSystem::glfw_shutdown();

    return 0;
}
