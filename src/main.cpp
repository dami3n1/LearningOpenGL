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
  Shader asteroidShader("../shaders/asteroidsShader.vert", "../shaders/asteroidsShader.frag");

  Model rock("../assets/rock/rock.obj");
  Model planet("../assets/planet/planet.obj");

  // generate a large list of semi-random model transformation matrices
  // ------------------------------------------------------------------
  unsigned int amount = 100000;
  glm::mat4 *modelMatrices;
  modelMatrices = new glm::mat4[amount];
  srand(static_cast<unsigned int>(glfwGetTime())); // initialize random seed
  float radius = 150.0;
  float offset = 25.0f;
  for (unsigned int i = 0; i < amount; i++) {
    glm::mat4 model = glm::mat4(1.0f);
    // 1. translation: displace along circle with 'radius' in range [-offset, offset]
    float angle = (float)i / (float)amount * 360.0f;
    float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
    float x = sin(angle) * radius + displacement;
    displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
    float y = displacement * 0.4f; // keep height of asteroid field smaller compared to width of x and z
    displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
    float z = cos(angle) * radius + displacement;
    model = glm::translate(model, glm::vec3(x, y, z));

    // 2. scale: Scale between 0.05 and 0.25f
    float scale = static_cast<float>((rand() % 20) / 100.0 + 0.05);
    model = glm::scale(model, glm::vec3(scale));

    // 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector
    float rotAngle = static_cast<float>((rand() % 360));
    model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

    // 4. now add to list of matrices
    modelMatrices[i] = model;
  }

  // configure instanced array
  // -------------------------
  unsigned int buffer;
  glGenBuffers(1, &buffer);
  glBindBuffer(GL_ARRAY_BUFFER, buffer);
  glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);

  // set up vertex data (and buffer(s)) and configure vertex attributes
  // ------------------------------------------------------------------
  for (unsigned int i = 0; i < rock.meshes.size(); i++) {
    unsigned int VAO = rock.meshes[i].VAO;
    glBindVertexArray(VAO);
    // set attribute pointers for matrix (4 times vec4)
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)0);
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(sizeof(glm::vec4)));
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(2 * sizeof(glm::vec4)));
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(3 * sizeof(glm::vec4)));

    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);
    glVertexAttribDivisor(5, 1);
    glVertexAttribDivisor(6, 1);

    glBindVertexArray(0);
  }
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
    glEnable(GL_CULL_FACE);

    glm::mat4 view = globalApplication::camera.GetViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(globalApplication::camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.001f, 10000.0f);

    asteroidShader.use();
    asteroidShader.setMat4("projection", projection);
    asteroidShader.setMat4("view", view);
    glm::mat4 asteroidOrbit = glm::rotate(glm::mat4(1.0f), currentFrame * 0.1f, glm::vec3(0.0f, 1.0f, 0.0f));
    asteroidShader.setMat4("orbit", asteroidOrbit);
    shader.use();
    shader.setMat4("projection", projection);
    shader.setMat4("view", view);

    // draw planet
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, -3.0f, 0.0f));
    model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
    shader.setMat4("model", model);
    planet.Draw(shader);

    // draw meteorites
    asteroidShader.use();
    asteroidShader.setInt("texture_diffuse1", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, rock.textures_loaded[0].id); // note: we also made the textures_loaded vector public (instead of private) from the model class.
    for (unsigned int i = 0; i < rock.meshes.size(); i++) {
      glBindVertexArray(rock.meshes[i].VAO);
      glDrawElementsInstanced(GL_TRIANGLES, static_cast<unsigned int>(rock.meshes[i].indices.size()), GL_UNSIGNED_INT, 0, amount);
      glBindVertexArray(0);
    }

    glfwSwapBuffers(globalApplication::window); // swaps color buffer in window
  }

  imguiLayer::Shutdown();

  windowSystem::glfw_shutdown();

  return 0;
}
