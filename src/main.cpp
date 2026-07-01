#include <cmath>
#include <iostream>
#include <iomanip>
#include <ostream>

#include "shader_reader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "controller.h"
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include "windowSystem.h"
#include "logger.h"
#include "imguiLayer.h"
#include <chrono>

#include "camera.h"
#include "global.h"
#include "model.h"
#include "inputHandler.h"

int SCREEN_HEIGHT = 600;
int SCREEN_WIDTH = 800;

unsigned int loadTexture(char const *path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            GLenum format;
            if (nrChannels == 1)
                format = GL_RED;
            else if (nrChannels == 2)
                format = GL_RG;
            else if (nrChannels == 3)
                format = GL_RGB;
            else
                format = GL_RGBA;

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            logger(ERROR, "Cubemap texture failed to load at path: " + faces[i]);
            stbi_image_free(data);
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

int main()
{
    if (!windowSystem::glfw_init())
    {
        logger(ERROR, "windowSystem::Failed to initialize GLFW");
        return -1; // or stop engine
    }
    globalApplication::window = windowSystem::makeWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "OpenGL Window");
    if (!globalApplication::window)
    {
        logger(ERROR, "windowSystem::Failed to create GLFW window");
        windowSystem::glfw_shutdown();
        return -1;
    }

    // disable vsync for uncapped framerate
    // glfwSwapInterval(0);

    globalApplication::input.setupMouse(globalApplication::window);

    // tell GLFW to capture our mouse
    glfwSetInputMode(globalApplication::window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // initialize GLAD before we can use any opengl functions
    // cast's the glfw function which gives the OS specific function for GLAD to find
    // the OpenGL function pointer (memory address of opengl executable command)(hardware specific)
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        logger(ERROR, "Failed to initialize GLAD");
        return -1;
    }

    glfwGetFramebufferSize(globalApplication::window, &SCREEN_WIDTH, &SCREEN_HEIGHT);
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    if (!imguiLayer::imguiSetup(globalApplication::window))
    {
        logger(ERROR, "Failed to initialize ImGui");
        return -1;
    }

    glEnable(GL_DEPTH_TEST); // enable depth testing for 3D

    Shader shader("../shaders/defaultShader.vert", "../shaders/defaultShader.frag");
    Shader reflectionShader("../shaders/reflectionShader.vert", "../shaders/reflectionShader.frag");
    Shader windowShader("../shaders/windowShader.vert", "../shaders/windowShader.frag");
    Shader framebuffershader("../shaders/framebufferShader.vert", "../shaders/framebufferShader.frag");
    Shader skyboxShader("../shaders/skyboxShader.vert", "../shaders/skyboxShader.frag");

    /*
    Remember: to specify vertices in a counter-clockwise winding order you need to visualize the triangle
    as if you're in front of the triangle and from that point of view, is where you set their order.

    To define the order of a triangle on the right side of the cube for example, you'd imagine yourself looking
    straight at the right side of the cube, and then visualize the triangle and make sure their order is specified
    in a counter-clockwise order. This takes some practice, but try visualizing this yourself and see that this
    is correct.
*/
    float cubeVertices[] = {
        // back (-Z)
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,

        // front (+Z)
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,

        // left (-X)
        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f,

        // right (+X)
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f,

        // bottom (-Y)
        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,

        // top (+Y)
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f};
    float planeVertices[] = {
        // positions          // texture Coords (note we set these higher than 1 (together with GL_REPEAT as texture wrapping mode). this will cause the floor texture to repeat)
        5.0f, -0.5f, 5.0f, 2.0f, 0.0f,
        -5.0f, -0.5f, 5.0f, 0.0f, 0.0f,
        -5.0f, -0.5f, -5.0f, 0.0f, 2.0f,

        5.0f, -0.5f, 5.0f, 2.0f, 0.0f,
        -5.0f, -0.5f, -5.0f, 0.0f, 2.0f,
        5.0f, -0.5f, -5.0f, 2.0f, 2.0f};
    float transparentVertices[] = {
        // positions         // texture Coords (swapped y coordinates because texture is flipped upside down)
        0.0f, 0.5f, 0.0f, 0.0f, 0.0f,
        0.0f, -0.5f, 0.0f, 0.0f, 1.0f,
        1.0f, -0.5f, 0.0f, 1.0f, 1.0f,

        0.0f, 0.5f, 0.0f, 0.0f, 0.0f,
        1.0f, -0.5f, 0.0f, 1.0f, 1.0f,
        1.0f, 0.5f, 0.0f, 1.0f, 0.0f};

    float quadVertices[] = {
        // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
        // positions   // texCoords
        -0.3f, 1.0f, 0.0f, 1.0f,
        -0.3f, 0.7f, 0.0f, 0.0f,
        0.3f, 0.7f, 1.0f, 0.0f,

        -0.3f, 1.0f, 0.0f, 1.0f,
        0.3f, 0.7f, 1.0f, 0.0f,
        0.3f, 1.0f, 1.0f, 1.0f};

    float skyboxVertices[] = {
        // positions
        -1.0f, 1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,

        -1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,

        -1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, 1.0f};

    vector<glm::vec3> windows{
        glm::vec3(-1.5f, 0.0f, -0.48f),
        glm::vec3(1.5f, 0.0f, 0.51f),
        glm::vec3(0.0f, 0.0f, 0.7f),
        glm::vec3(-0.3f, 0.0f, -2.3f),
        glm::vec3(0.5f, 0.0f, -0.6f)};
    // cube VAO
    unsigned int cubeVAO, cubeVBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    glBindVertexArray(0);
    // plane VAO
    unsigned int planeVAO, planeVBO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glBindVertexArray(0);
    // transparent VAO
    unsigned int transparentVAO, transparentVBO;
    glGenVertexArrays(1, &transparentVAO);
    glGenBuffers(1, &transparentVBO);
    glBindVertexArray(transparentVAO);
    glBindBuffer(GL_ARRAY_BUFFER, transparentVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(transparentVertices), transparentVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glBindVertexArray(0);
    // quad VAO
    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glBindVertexArray(0);
    // skybox VAO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glBindVertexArray(0);

    unsigned int floorTexture = loadTexture("../assets/metal.png");
    unsigned int transparentTexture = loadTexture("../assets/blending_transparent_window.png");

    auto t0 = std::chrono::high_resolution_clock::now();
    Model backpack("../assets/backpack/backpack.obj");
    auto t1 = std::chrono::high_resolution_clock::now();

    std::cout << "Total load: "
              << std::chrono::duration<double>(t1 - t0).count()
              << "s\n";

    vector<std::string> faces{
        "../assets/skybox2/right.png",
        "../assets/skybox2/left.png",
        "../assets/skybox2/top.png",
        "../assets/skybox2/bottom.png",
        "../assets/skybox2/front.png",
        "../assets/skybox2/back.png"};
    unsigned int cubemapTexture = loadCubemap(faces);

    globalApplication::controller.showControllers();

    float lastFrame = 0.0f;

    // Unbind the VBO (optional, just to avoid accidental changes later)
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    int width = SCREEN_WIDTH, height = SCREEN_HEIGHT;
    int framebufferWidth = SCREEN_WIDTH;
    int framebufferHeight = SCREEN_HEIGHT;

    glEnable(GL_DEPTH_TEST); // enable depth testing for 3D
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    shader.use();
    shader.setInt("texture1", 0);

    reflectionShader.use();
    reflectionShader.setInt("skybox", 0);

    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    windowShader.use();
    windowShader.setInt("windowTexture", 0);

    framebuffershader.use();
    framebuffershader.setInt("screenTexture", 0);

    // create framebuffer and bind it
    unsigned int framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    // create a texture image and attach color attachment to framebuffer
    // generate texture
    unsigned int textureColorbuffer;
    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    // attach it to currently bound framebuffer object
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

    // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    // precision of 24 bits for depth and 8 bits for stencil
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCREEN_WIDTH, SCREEN_HEIGHT);
    // unbind renderbuffer object after we set its storage so that we won't accidentally mess with it
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // attach the renderbuffer object to the framebuffer's depth and stencil attachment
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    // check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        logger(ERROR, "ERROR::FRAMEBUFFER:: Framebuffer is not complete!");

    // unbind framebuffer so that we can render to default framebuffer again
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Now we draw to our framebuffer object then swap to main buffer to display what was on our object
    // This allows us to do post processing effects and other cool things by sampling the texture we attached
    // to our framebuffer object and applying effects to it before we draw it to the main framebuffer

    // draw as wireframe
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    while (!glfwWindowShouldClose(globalApplication::window))
    {
        if (globalApplication::controller.X())
        {
            shader.reload();
        }
        glfwPollEvents(); // processes events received in window and returns a response(if requested)
        // input function called each frame
        inputHandler::processInput(globalApplication::window);

        float currentFrame = static_cast<float>(glfwGetTime());
        globalApplication::input.deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwGetFramebufferSize(globalApplication::window, &width, &height);

        if (width != framebufferWidth || height != framebufferHeight)
        {
            framebufferWidth = width;
            framebufferHeight = height;

            // Resize the off-screen color attachment.
            glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, framebufferWidth, framebufferHeight,
                         0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
            glBindTexture(GL_TEXTURE_2D, 0);

            // Resize its matching depth/stencil attachment.
            glBindRenderbuffer(GL_RENDERBUFFER, rbo);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8,
                                  framebufferWidth, framebufferHeight);
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
        }

        SCREEN_WIDTH = width;
        SCREEN_HEIGHT = height;
        glViewport(0, 0, framebufferWidth, framebufferHeight);

        globalApplication::controller.update();
        globalApplication::controller.processController();

        std::map<float, glm::vec3> sorted;
        for (unsigned int i = 0; i < windows.size(); i++)
        {
            float distance = glm::length(globalApplication::camera.Position - windows[i]);
            sorted[distance] = windows[i];
        }

        // PASS 1: OFF-SCREEN FRAMEBUFFER
        // Draw the reversed-camera scene into textureColorbuffer.
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glEnable(GL_DEPTH_TEST); // enable depth testing for 3D

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // glEnable(GL_DEPTH_TEST); // enable depth testing for 3D
        glEnable(GL_CULL_FACE);

        glm::mat4 model = glm::mat4(1.0f);

        // The preview quad is twice as wide as it is tall in NDC. Account for
        // that here so the mirror image is not stretched inside the box.
        glm::mat4 projection = glm::perspective(
            glm::radians(globalApplication::camera.Zoom),
            2.0f * (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT,
            0.1f,
            100.0f);

        glm::mat4 view = glm::lookAt(
            globalApplication::camera.Position,
            globalApplication::camera.Position - globalApplication::camera.Front,
            globalApplication::camera.Up);
        glm::mat4 skyboxView = glm::mat4(glm::mat3(view));

        // Shader setup for the entire off-screen pass.
        shader.use();
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);

        reflectionShader.use();
        reflectionShader.setMat4("view", view);
        reflectionShader.setMat4("projection", projection);
        reflectionShader.setVec3("cameraPos", globalApplication::camera.Position);

        skyboxShader.use();
        skyboxShader.setMat4("view", skyboxView);
        skyboxShader.setMat4("projection", projection);

        windowShader.use();
        windowShader.setMat4("view", view);
        windowShader.setMat4("projection", projection);

        glm::vec3 positions[2] = {
            glm::vec3(-1.0f, 0.0f, -1.0f),
            glm::vec3(2.0f, 0.0f, 0.0f)};

        // Reflective cube shader.
        reflectionShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glBindVertexArray(cubeVAO);
        for (int i = 0; i < 2; i++)
        {
            glm::vec3 pos = positions[i];

            model = glm::mat4(1.0f);
            model = glm::translate(model, pos);
            reflectionShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        glBindVertexArray(0);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.02f, 1.5f));
        model = glm::scale(model, glm::vec3(0.3f));
        reflectionShader.setMat4("model", model);
        backpack.Draw(reflectionShader);

        glDisable(GL_CULL_FACE);

        // Floor shader.
        shader.use();
        glBindVertexArray(planeVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, floorTexture);
        shader.setMat4("model", glm::mat4(1.0f));
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        // Skybox shader.
        glDepthFunc(GL_LEQUAL); // set depth function to less than (default)
        skyboxShader.use();
        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default

        // Transparent-window shader.
        windowShader.use();
        glDepthMask(GL_FALSE);
        glBindVertexArray(transparentVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, transparentTexture);
        // reverse iterator so we draw the ones behind first before the ones in front
        for (std::map<float, glm::vec3>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, it->second);
            windowShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        glBindVertexArray(0);
        glDepthMask(GL_TRUE);

        // PASS 2: REGULAR SCREEN
        // Framebuffer 0 is the window's default framebuffer.
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST); // enable depth testing for 3D
        glEnable(GL_CULL_FACE);

        model = glm::mat4(1.0f);
        view = globalApplication::camera.GetViewMatrix();
        projection = glm::perspective(
            glm::radians(globalApplication::camera.Zoom),
            (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT,
            0.1f,
            100.0f);
        skyboxView = glm::mat4(glm::mat3(view));

        // Shader setup for the entire regular-screen pass.
        shader.use();
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);

        reflectionShader.use();
        reflectionShader.setMat4("view", view);
        reflectionShader.setMat4("projection", projection);
        reflectionShader.setVec3("cameraPos", globalApplication::camera.Position);

        skyboxShader.use();
        skyboxShader.setMat4("view", skyboxView);
        skyboxShader.setMat4("projection", projection);

        windowShader.use();
        windowShader.setMat4("view", view);
        windowShader.setMat4("projection", projection);

        // Reflective cube shader.
        reflectionShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glBindVertexArray(cubeVAO);
        for (int i = 0; i < 2; i++)
        {
            glm::vec3 pos = positions[i];

            model = glm::mat4(1.0f);
            model = glm::translate(model, pos);
            reflectionShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        glBindVertexArray(0);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.02f, 1.5f));
        model = glm::scale(model, glm::vec3(0.3f));
        reflectionShader.setMat4("model", model);
        backpack.Draw(reflectionShader);

        glDisable(GL_CULL_FACE);

        // Floor shader.
        shader.use();
        glBindVertexArray(planeVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, floorTexture);
        shader.setMat4("model", glm::mat4(1.0f));
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        // Skybox shader.
        glDepthFunc(GL_LEQUAL); // set depth function to less than (default)
        skyboxShader.use();
        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default

        // Transparent-window shader.
        windowShader.use();
        glDepthMask(GL_FALSE);
        glBindVertexArray(transparentVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, transparentTexture);
        // reverse iterator so we draw the ones behind first before the ones in front
        for (std::map<float, glm::vec3>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, it->second);
            windowShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        glBindVertexArray(0);
        glDepthMask(GL_TRUE);

        // FRAMEBUFFER PREVIEW
        // Draw textureColorbuffer onto the small screen-space quad.
        glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.
        framebuffershader.use();
        glBindVertexArray(quadVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Rendering
        // (Your code clears your framebuffer, renders your other stuff etc.)
        // ImGui::Render();
        // ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        // (Your code calls glfwSwapBuffers() etc.)

        glfwSwapBuffers(globalApplication::window); // swaps color buffer in window
    }

    imguiLayer::Shutdown();

    // de allocated resources after usage
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &planeVAO);
    glDeleteBuffers(1, &cubeVBO);
    glDeleteBuffers(1, &planeVBO);

    windowSystem::glfw_shutdown();

    return 0;
}
