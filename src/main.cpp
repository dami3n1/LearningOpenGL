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
#include <chrono>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_impl_opengl3.h>

glm::vec3 lightPos(1.2f, 0.0f, 2.0f);

struct DirectionalLight {
  glm::vec3 direction = glm::vec3(-0.2f, -1.0f, -0.3f);

  glm::vec3 ambient = glm::vec3(0.05f);
  glm::vec3 diffuse = glm::vec3(0.4f);
  glm::vec3 specular = glm::vec3(0.5f);

  bool enabled = true;
};

struct PointLight {
  glm::vec3 position;

  glm::vec3 ambient = glm::vec3(0.05f);
  glm::vec3 diffuse = glm::vec3(0.8f);
  glm::vec3 specular = glm::vec3(1.0f);

  float constant = 1.0f;
  float linear = 0.09f;
  float quadratic = 0.032f;

  bool enabled = true;
};

struct SpotLight {
  glm::vec3 ambient = glm::vec3(0.0f);
  glm::vec3 diffuse = glm::vec3(1.0f);
  glm::vec3 specular = glm::vec3(1.0f);

  float constant = 1.0f;
  float linear = 0.09f;
  float quadratic = 0.032f;

  float cutOff = 12.5f;
  float outerCutOff = 15.0f;

  bool enabled = false;
};

DirectionalLight dirLight;

PointLight pointLights[4] = {{glm::vec3(0.7f, 0.2f, 2.0f)}, {glm::vec3(2.3f, -3.3f, -4.0f)}, {glm::vec3(-4.0f, 2.0f, -12.0f)}, {glm::vec3(0.0f, 0.0f, -3.0f)}};

SpotLight spotLight;

glm::vec3 emissionColor = glm::vec3(0.0f, 0.0f, 1.0f);
float emissionStrength = 0.0f;
float shininess = 32.0f;

int SCREEN_HEIGHT = 600;
int SCREEN_WIDTH = 800;

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

  Shader shader("../shaders/defaultShader.vert", "../shaders/defaultShader.frag");

  // generate a list of 100 quad locations/translation-vectors
  // ---------------------------------------------------------
  glm::vec2 translations[100];
  int index = 0;
  float offset = 0.1f;
  for (int y = -10; y < 10; y += 2) {
    for (int x = -10; x < 10; x += 2) {
      glm::vec2 translation;
      translation.x = (float)x / 10.0f + offset;
      translation.y = (float)y / 10.0f + offset;
      translations[index++] = translation;
    }
  }

  // store instance data in an array buffer
  // --------------------------------------
  unsigned int instanceVBO;
  glGenBuffers(1, &instanceVBO);
  glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * 100, &translations[0], GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // set up vertex data (and buffer(s)) and configure vertex attributes
  // ------------------------------------------------------------------
  float quadVertices[] = {// positions     // colors
                          -0.05f, 0.05f, 1.0f, 0.0f, 0.0f, 0.05f, -0.05f, 0.0f, 1.0f, 0.0f, -0.05f, -0.05f, 0.0f, 0.0f, 1.0f,

                          -0.05f, 0.05f, 1.0f, 0.0f, 0.0f, 0.05f, -0.05f, 0.0f, 1.0f, 0.0f, 0.05f,  0.05f,  0.0f, 1.0f, 1.0f};
  unsigned int quadVAO, quadVBO;
  glGenVertexArrays(1, &quadVAO);
  glGenBuffers(1, &quadVBO);
  glBindVertexArray(quadVAO);
  glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(2 * sizeof(float)));
  // also set instance data
  glEnableVertexAttribArray(2);
  glBindBuffer(GL_ARRAY_BUFFER, instanceVBO); // this attribute comes from a different vertex buffer
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glVertexAttribDivisor(2, 1); // tell OpenGL this is an instanced vertex attribute.

  globalApplication::controller.showControllers();

  float lastFrame = 0.0f;

  // Unbind the VBO (optional, just to avoid accidental changes later)
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  int width = SCREEN_WIDTH, height = SCREEN_HEIGHT;

  glEnable(GL_DEPTH_TEST); // enable depth testing for 3D
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  shader.use();
  shader.setInt("texture1", 0);

  // draw as wireframe
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  // render loop
  while (!glfwWindowShouldClose(globalApplication::window)) {
    if (globalApplication::controller.X()) {
      shader.reload();
    }
    glfwPollEvents(); // processes events received in window and returns a
                      // response(if requested)
    // input function called each frame
    inputHandler::processInput(globalApplication::window);

    float currentFrame = static_cast<float>(glfwGetTime());
    globalApplication::input.deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    glfwGetFramebufferSize(globalApplication::window, &width, &height);
    SCREEN_WIDTH = width;
    SCREEN_HEIGHT = height;
    glViewport(0, 0, width, height);

    globalApplication::controller.update();
    globalApplication::controller.processController();

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shader.use();
    glBindVertexArray(quadVAO);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 100);
    glBindVertexArray(0);

    glfwSwapBuffers(globalApplication::window); // swaps color buffer in window
  }

  imguiLayer::Shutdown();

  // de allocated resources after usage
  glDeleteVertexArrays(1, &quadVAO);
  glDeleteBuffers(1, &quadVBO);

  windowSystem::glfw_shutdown();

  return 0;
}
