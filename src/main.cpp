#include <iostream>
#include <ostream>

#include "camera.h"
#include "controller.h"
#include "global.h"
#include "imguiLayer.h"
#include "inputHandler.h"
#include "logger.h"
#include "model.h"
#include "shader_reader.h"
#include "windowSystem.h"
#include <array>
#include <chrono>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_impl_opengl3.h>

int SCREEN_HEIGHT = 600;
int SCREEN_WIDTH = 800;

unsigned int loadTexture(char const *path) {
  unsigned int textureID;
  glGenTextures(1, &textureID);

  int width, height, nrComponents;
  unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
  if (data) {
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

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
  } else {
    std::cout << "Texture failed to load at path: " << path << std::endl;
    stbi_image_free(data);
  }

  return textureID;
}

int main() {
  if (!windowSystem::glfw_init()) {
    logger(ERROR, "windowSystem::Failed to initialize GLFW");
    return -1; // or stop engine
  }
  globalApplication::window = windowSystem::makeWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "OpenGL Window");
  if (!globalApplication::window) {
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
  // cast's the glfw function which gives the OS specific function for GLAD to
  // find the OpenGL function pointer (memory address of opengl executable
  // command)(hardware specific)
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    logger(ERROR, "Failed to initialize GLAD");
    return -1;
  }

  glfwGetFramebufferSize(globalApplication::window, &SCREEN_WIDTH, &SCREEN_HEIGHT);
  glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

  if (!imguiLayer::imguiSetup(globalApplication::window)) {
    logger(ERROR, "Failed to initialize ImGui");
    return -1;
  }

  glEnable(GL_DEPTH_TEST); // enable depth testing for 3D

  globalApplication::controller.showControllers();

  float lastFrame = 0.0f;

  // build and compile shaders
  // -------------------------
  Shader shader("../shaders/illuminateShader.vert", "../shaders/illuminateShader.frag");

  // set up vertex data (and buffer(s)) and configure vertex attributes
  // ------------------------------------------------------------------
  float planeVertices[] = {// positions            // normals         // texcoords
                           10.0f, -0.5f, 10.0f, 0.0f, 1.0f, 0.0f, 10.0f, 0.0f, -10.0f, -0.5f, 10.0f,  0.0f, 1.0f, 0.0f, 0.0f, 0.0f,  -10.0f, -0.5f, -10.0f, 0.0f, 1.0f, 0.0f, 0.0f,  10.0f,

                           10.0f, -0.5f, 10.0f, 0.0f, 1.0f, 0.0f, 10.0f, 0.0f, -10.0f, -0.5f, -10.0f, 0.0f, 1.0f, 0.0f, 0.0f, 10.0f, 10.0f,  -0.5f, -10.0f, 0.0f, 1.0f, 0.0f, 10.0f, 10.0f};
  // plane VAO
  unsigned int planeVAO, planeVBO;
  glGenVertexArrays(1, &planeVAO);
  glGenBuffers(1, &planeVBO);
  glBindVertexArray(planeVAO);
  glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
  glBindVertexArray(0);

  // load textures
  // -------------
  unsigned int floorTexture = loadTexture("../assets/wood.png");

  // shader configuration
  // --------------------
  shader.use();
  shader.setInt("material.diffuse", 0);
  shader.setInt("material.specular", 0);
  shader.setInt("material.emission", 0);
  shader.setFloat("material.shininess", 32.0f);
  shader.setFloat("material.emissionStrength", 0.0f);
  shader.setVec3("material.emissionColor", glm::vec3(0.0f));
  shader.setMat4("model", glm::mat4(1.0f));

  // A cool directional fill light and four colored point lights make the
  // Blinn-Phong highlights easy to inspect across the floor.
  shader.setVec3("dirLight.direction", glm::vec3(-0.2f, -1.0f, -0.3f));
  shader.setVec3("dirLight.ambient", glm::vec3(0.04f));
  shader.setVec3("dirLight.diffuse", glm::vec3(0.18f, 0.20f, 0.24f));
  shader.setVec3("dirLight.specular", glm::vec3(0.35f));

  const std::array<glm::vec3, 4> pointLightPositions = {
      glm::vec3(-4.0f, 1.5f, -3.0f), glm::vec3(4.0f, 2.0f, -2.0f),
      glm::vec3(-3.0f, 1.0f, 4.0f), glm::vec3(3.5f, 1.5f, 4.5f)};
  const std::array<glm::vec3, 4> pointLightColors = {
      glm::vec3(1.0f, 0.25f, 0.15f), glm::vec3(0.15f, 0.35f, 1.0f),
      glm::vec3(0.20f, 1.0f, 0.35f), glm::vec3(1.0f, 0.65f, 0.15f)};

  for (std::size_t i = 0; i < pointLightPositions.size(); ++i) {
    const std::string light = "pointLights[" + std::to_string(i) + "]";
    shader.setVec3(light + ".position", pointLightPositions[i]);
    shader.setVec3(light + ".ambient", pointLightColors[i] * 0.01f);
    shader.setVec3(light + ".diffuse", pointLightColors[i] * 0.75f);
    shader.setVec3(light + ".specular", pointLightColors[i]);
    shader.setFloat(light + ".constant", 1.0f);
    shader.setFloat(light + ".linear", 0.09f);
    shader.setFloat(light + ".quadratic", 0.032f);
  }

  // The spotlight follows the camera like a flashlight.
  shader.setVec3("spotLight.ambient", glm::vec3(0.0f));
  shader.setVec3("spotLight.diffuse", glm::vec3(0.65f));
  shader.setVec3("spotLight.specular", glm::vec3(1.0f));
  shader.setFloat("spotLight.constant", 1.0f);
  shader.setFloat("spotLight.linear", 0.09f);
  shader.setFloat("spotLight.quadratic", 0.032f);
  shader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
  shader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(17.5f)));

  int width = SCREEN_WIDTH, height = SCREEN_HEIGHT;

  // draw as wireframe
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  //

  // render loop
  while (!glfwWindowShouldClose(globalApplication::window)) {
    glfwPollEvents(); // processes events received in window and returns a
                      // response(if requested)
    // input function called each frame
    inputHandler::processInput(globalApplication::window);

    float currentFrame = static_cast<float>(glfwGetTime());
    globalApplication::input.deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // render
    // ------
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // draw objects
    shader.use();
    glm::mat4 projection = glm::perspective(glm::radians(globalApplication::camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = globalApplication::camera.GetViewMatrix();
    shader.setMat4("projection", projection);
    shader.setMat4("view", view);
    // set light uniforms
    shader.setVec3("viewPos", globalApplication::camera.Position);
    shader.setVec3("spotLight.position", globalApplication::camera.Position);
    shader.setVec3("spotLight.direction", globalApplication::camera.Front);
    // floor
    glBindVertexArray(planeVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, floorTexture);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glfwGetFramebufferSize(globalApplication::window, &width, &height);

    // A minimized window has no drawable framebuffer.
    if (width == 0 || height == 0)
      continue;

    globalApplication::controller.update();
    globalApplication::controller.processController();

    glfwSwapBuffers(globalApplication::window); // swaps color buffer in window
  }

  imguiLayer::Shutdown();

  windowSystem::glfw_shutdown();

  return 0;
}
