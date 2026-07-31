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

PointLight pointLights[4] = {{glm::vec3(0.7f, 0.2f, 2.0f)},
                             {glm::vec3(2.3f, -3.3f, -4.0f)},
                             {glm::vec3(-4.0f, 2.0f, -12.0f)},
                             {glm::vec3(0.0f, 0.0f, -3.0f)}};

SpotLight spotLight;

glm::vec3 emissionColor = glm::vec3(0.0f, 0.0f, 1.0f);
float emissionStrength = 0.0f;
float shininess = 32.0f;

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
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                    format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                    format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
  } else {
    std::cout << "Texture failed to load at path: " << path << std::endl;
    stbi_image_free(data);
  }

  return textureID;
}

unsigned int loadCubemap(vector<std::string> faces) {
  unsigned int textureID;
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

  int width, height, nrChannels;
  for (unsigned int i = 0; i < faces.size(); i++) {
    unsigned char *data =
        stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
    if (data) {
      GLenum format;
      if (nrChannels == 1)
        format = GL_RED;
      else if (nrChannels == 2)
        format = GL_RG;
      else if (nrChannels == 3)
        format = GL_RGB;
      else
        format = GL_RGBA;

      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height,
                   0, format, GL_UNSIGNED_BYTE, data);
      stbi_image_free(data);
    } else {
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

int main() {
  if (!windowSystem::glfw_init()) {
    logger(ERROR, "windowSystem::Failed to initialize GLFW");
    return -1; // or stop engine
  }
  globalApplication::window =
      windowSystem::makeWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "OpenGL Window");
  if (!globalApplication::window) {
    logger(ERROR, "windowSystem::Failed to create GLFW window");
    windowSystem::glfw_shutdown();
    return -1;
  }

  // disable vsync for uncapped framerate
  // glfwSwapInterval(0);

  globalApplication::input.setupMouse(globalApplication::window);

  // tell GLFW to capture our mouse
  glfwSetInputMode(globalApplication::window, GLFW_CURSOR,
                   GLFW_CURSOR_DISABLED);

  // initialize GLAD before we can use any opengl functions
  // cast's the glfw function which gives the OS specific function for GLAD to
  // find the OpenGL function pointer (memory address of opengl executable
  // command)(hardware specific)
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    logger(ERROR, "Failed to initialize GLAD");
    return -1;
  }

  glfwGetFramebufferSize(globalApplication::window, &SCREEN_WIDTH,
                         &SCREEN_HEIGHT);
  glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

  if (!imguiLayer::imguiSetup(globalApplication::window)) {
    logger(ERROR, "Failed to initialize ImGui");
    return -1;
  }

  glEnable(GL_DEPTH_TEST); // enable depth testing for 3D

  Shader shader("../shaders/defaultShader.vert",
                "../shaders/defaultShader.frag");
  Shader reflectionShader("../shaders/refractionShader.vert",
                          "../shaders/refractionShader.frag");
  Shader windowShader("../shaders/windowShader.vert",
                      "../shaders/windowShader.frag");
  Shader skyboxShader("../shaders/skyboxShader.vert",
                      "../shaders/skyboxShader.frag");
  Shader lightCubeShader("../shaders/light_cube.vert",
                         "../shaders/light_cube.frag");
  Shader lightingShader("../shaders/illuminateShader.vert",
                        "../shaders/illuminateShader.frag");
  // for ubo example
  Shader shaderRed("../shaders/uboshader.vert", "../shaders/red.frag");
  Shader shaderGreen("../shaders/uboshader.vert", "../shaders/green.frag");
  Shader shaderBlue("../shaders/uboshader.vert", "../shaders/blue.frag");
  Shader shaderYellow("../shaders/uboshader.vert", "../shaders/yellow.frag");

  /*
  Remember: to specify vertices in a counter-clockwise winding order you need to
  visualize the triangle as if you're in front of the triangle and from that
  point of view, is where you set their order.

  To define the order of a triangle on the right side of the cube for example,
  you'd imagine yourself looking straight at the right side of the cube, and
  then visualize the triangle and make sure their order is specified in a
  counter-clockwise order. This takes some practice, but try visualizing this
  yourself and see that this is correct.
*/
  float cubeVertices[] = {
      // Back face
      -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, // Bottom-left
      0.5f, 0.5f, -0.5f, 1.0f, 1.0f,   // top-right
      0.5f, -0.5f, -0.5f, 1.0f, 0.0f,  // bottom-right
      0.5f, 0.5f, -0.5f, 1.0f, 1.0f,   // top-right
      -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, // bottom-left
      -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,  // top-left
      // Front face
      -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, // bottom-left
      0.5f, -0.5f, 0.5f, 1.0f, 0.0f,  // bottom-right
      0.5f, 0.5f, 0.5f, 1.0f, 1.0f,   // top-right
      0.5f, 0.5f, 0.5f, 1.0f, 1.0f,   // top-right
      -0.5f, 0.5f, 0.5f, 0.0f, 1.0f,  // top-left
      -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, // bottom-left
      // Left face
      -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,   // top-right
      -0.5f, 0.5f, -0.5f, 1.0f, 1.0f,  // top-left
      -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, // bottom-left
      -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, // bottom-left
      -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,  // bottom-right
      -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,   // top-right
                                       // Right face
      0.5f, 0.5f, 0.5f, 1.0f, 0.0f,    // top-left
      0.5f, -0.5f, -0.5f, 0.0f, 1.0f,  // bottom-right
      0.5f, 0.5f, -0.5f, 1.0f, 1.0f,   // top-right
      0.5f, -0.5f, -0.5f, 0.0f, 1.0f,  // bottom-right
      0.5f, 0.5f, 0.5f, 1.0f, 0.0f,    // top-left
      0.5f, -0.5f, 0.5f, 0.0f, 0.0f,   // bottom-left
      // Bottom face
      -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, // top-right
      0.5f, -0.5f, -0.5f, 1.0f, 1.0f,  // top-left
      0.5f, -0.5f, 0.5f, 1.0f, 0.0f,   // bottom-left
      0.5f, -0.5f, 0.5f, 1.0f, 0.0f,   // bottom-left
      -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,  // bottom-right
      -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, // top-right
      // Top face
      -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, // top-left
      0.5f, 0.5f, 0.5f, 1.0f, 0.0f,   // bottom-right
      0.5f, 0.5f, -0.5f, 1.0f, 1.0f,  // top-right
      0.5f, 0.5f, 0.5f, 1.0f, 0.0f,   // bottom-right
      -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, // top-left
      -0.5f, 0.5f, 0.5f, 0.0f, 0.0f   // bottom-left
  };
  float regularCube[] = {
      // Back face (-Z)
      0.5f,
      -0.5f,
      -0.5f,
      -0.5f,
      -0.5f,
      -0.5f,
      -0.5f,
      0.5f,
      -0.5f,
      -0.5f,
      0.5f,
      -0.5f,
      0.5f,
      0.5f,
      -0.5f,
      0.5f,
      -0.5f,
      -0.5f,

      // Front face (+Z)
      -0.5f,
      -0.5f,
      0.5f,
      0.5f,
      -0.5f,
      0.5f,
      0.5f,
      0.5f,
      0.5f,
      0.5f,
      0.5f,
      0.5f,
      -0.5f,
      0.5f,
      0.5f,
      -0.5f,
      -0.5f,
      0.5f,

      // Left face (-X)
      -0.5f,
      -0.5f,
      -0.5f,
      -0.5f,
      -0.5f,
      0.5f,
      -0.5f,
      0.5f,
      0.5f,
      -0.5f,
      0.5f,
      0.5f,
      -0.5f,
      0.5f,
      -0.5f,
      -0.5f,
      -0.5f,
      -0.5f,

      // Right face (+X)
      0.5f,
      -0.5f,
      0.5f,
      0.5f,
      -0.5f,
      -0.5f,
      0.5f,
      0.5f,
      -0.5f,
      0.5f,
      0.5f,
      -0.5f,
      0.5f,
      0.5f,
      0.5f,
      0.5f,
      -0.5f,
      0.5f,

      // Bottom face (-Y)
      -0.5f,
      -0.5f,
      -0.5f,
      0.5f,
      -0.5f,
      -0.5f,
      0.5f,
      -0.5f,
      0.5f,
      0.5f,
      -0.5f,
      0.5f,
      -0.5f,
      -0.5f,
      0.5f,
      -0.5f,
      -0.5f,
      -0.5f,

      // Top face (+Y)
      -0.5f,
      0.5f,
      0.5f,
      0.5f,
      0.5f,
      0.5f,
      0.5f,
      0.5f,
      -0.5f,
      0.5f,
      0.5f,
      -0.5f,
      -0.5f,
      0.5f,
      -0.5f,
      -0.5f,
      0.5f,
      0.5f,
  };
  float planeVertices[] = {
      // positions          // texture Coords (note we set these higher than 1
      // (together with GL_REPEAT as texture wrapping mode). this will cause the
      // floor texture to repeat)
      5.0f, -0.5f, 5.0f, 2.0f, 0.0f, -5.0f, -0.5f, 5.0f,
      0.0f, 0.0f, -5.0f, -0.5f, -5.0f, 0.0f, 2.0f,

      5.0f, -0.5f, 5.0f, 2.0f, 0.0f, -5.0f, -0.5f, -5.0f,
      0.0f, 2.0f, 5.0f, -0.5f, -5.0f, 2.0f, 2.0f};
  float transparentVertices[] = {
      // positions         // texture Coords (swapped y coordinates because
      // texture is flipped upside down)
      0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, -0.5f, 0.0f,
      0.0f, 1.0f, 1.0f, -0.5f, 0.0f, 1.0f, 1.0f,

      0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, -0.5f, 0.0f,
      1.0f, 1.0f, 1.0f, 0.5f, 0.0f, 1.0f, 0.0f};

  float skyboxVertices[] = {
      // positions
      -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f,
      1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f,

      -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f,
      -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f,

      1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
      1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f,

      -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
      1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f,

      -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f,
      1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f,

      -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f,
      1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f};

  float vertices[] = {
      // positions
      -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, 0.5f, -0.5f,
      0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, -0.5f, -0.5f,

      -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f, 0.5f,
      0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f,

      -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, -0.5f, -0.5f, -0.5f, -0.5f,
      -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,

      0.5f, 0.5f, 0.5f, 0.5f, 0.5f, -0.5f, 0.5f, -0.5f, -0.5f,
      0.5f, -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f, 0.5f,

      -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f, 0.5f,
      0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, -0.5f,

      -0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f,
      0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, -0.5f};

  vector<glm::vec3> windows{
      glm::vec3(-1.5f, 0.0f, -0.48f), glm::vec3(1.5f, 0.0f, 0.51f),
      glm::vec3(0.0f, 0.0f, 0.7f), glm::vec3(-0.3f, 0.0f, -2.3f),
      glm::vec3(0.5f, 0.0f, -0.6f)};

  // first, configure the cube's VAO (and VBO)
  unsigned int VBO, lightCubeVAO;
  glGenVertexArrays(1, &lightCubeVAO);
  glGenBuffers(1, &VBO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glBindVertexArray(lightCubeVAO);

  // note that we update the lamp's position attribute's stride to reflect the
  // updated buffer data
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  // cube VAO
  unsigned int cubeVAO, cubeVBO;
  glGenVertexArrays(1, &cubeVAO);
  glGenBuffers(1, &cubeVBO);
  glBindVertexArray(cubeVAO);
  glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices,
               GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glBindVertexArray(0);
  // plane VAO
  unsigned int planeVAO, planeVBO;
  glGenVertexArrays(1, &planeVAO);
  glGenBuffers(1, &planeVBO);
  glBindVertexArray(planeVAO);
  glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices,
               GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glBindVertexArray(0);
  // transparent VAO
  unsigned int transparentVAO, transparentVBO;
  glGenVertexArrays(1, &transparentVAO);
  glGenBuffers(1, &transparentVBO);
  glBindVertexArray(transparentVAO);
  glBindBuffer(GL_ARRAY_BUFFER, transparentVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(transparentVertices),
               transparentVertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glBindVertexArray(0);
  // skybox VAO
  unsigned int skyboxVAO, skyboxVBO;
  glGenVertexArrays(1, &skyboxVAO);
  glGenBuffers(1, &skyboxVBO);
  glBindVertexArray(skyboxVAO);
  glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices,
               GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glBindVertexArray(0);
  // regularcube VAO
  unsigned int regularCubeVAO, regularCubeVBO;
  glGenVertexArrays(1, &regularCubeVAO);
  glGenBuffers(1, &regularCubeVBO);
  glBindVertexArray(regularCubeVAO);
  glBindBuffer(GL_ARRAY_BUFFER, regularCubeVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(regularCube), &regularCube,
               GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

  unsigned int uniformBlockIndexRed =
      glGetUniformBlockIndex(shaderRed.ID, "Matrices");
  unsigned int uniformBlockIndexGreen =
      glGetUniformBlockIndex(shaderGreen.ID, "Matrices");
  unsigned int uniformBlockIndexBlue =
      glGetUniformBlockIndex(shaderBlue.ID, "Matrices");
  unsigned int uniformBlockIndexYellow =
      glGetUniformBlockIndex(shaderYellow.ID, "Matrices");

  glUniformBlockBinding(shaderRed.ID, uniformBlockIndexRed, 0);
  glUniformBlockBinding(shaderGreen.ID, uniformBlockIndexGreen, 0);
  glUniformBlockBinding(shaderBlue.ID, uniformBlockIndexBlue, 0);
  glUniformBlockBinding(shaderYellow.ID, uniformBlockIndexYellow, 0);

  // uniform glBufferobject for lighting
  unsigned int uboMatricies;
  glGenBuffers(1, &uboMatricies);
  glBindBuffer(GL_UNIFORM_BUFFER, uboMatricies);
  glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);

  glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboMatricies, 0,
                    2 * sizeof(glm::mat4));

  unsigned int floorTexture = loadTexture("../assets/metal.png");
  unsigned int cubeTexture = loadTexture("../assets/container.jpg");
  unsigned int transparentTexture =
      loadTexture("../assets/blending_transparent_window.png");

  auto t0 = std::chrono::high_resolution_clock::now();
  Model backpack("../assets/blackpink.obj", false, true);
  auto t1 = std::chrono::high_resolution_clock::now();

  std::cout << "Total load: " << std::chrono::duration<double>(t1 - t0).count()
            << "s\n";

  vector<std::string> faces{
      "../assets/skybox2/right.png", "../assets/skybox2/left.png",
      "../assets/skybox2/top.png", "../assets/skybox2/bottom.png",
      "../assets/skybox2/front.png", "../assets/skybox2/back.png"};
  unsigned int cubemapTexture = loadCubemap(faces);

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

  reflectionShader.use();
  reflectionShader.setInt("skybox", 0);

  skyboxShader.use();
  skyboxShader.setInt("skybox", 0);

  windowShader.use();
  windowShader.setInt("windowTexture", 0);

  // draw as wireframe
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  // render loop
  while (!glfwWindowShouldClose(globalApplication::window)) {

    imguiLayer::imguiRender();
    ImGui::NewFrame();

    {
      ImGui::Begin("Lighting Editor");

      ImGui::Text("Directional Light");

      ImGui::Checkbox("Enable Dir Light", &dirLight.enabled);

      ImGui::DragFloat3("Dir Direction", glm::value_ptr(dirLight.direction),
                        0.1f);

      ImGui::ColorEdit3("Dir Ambient", glm::value_ptr(dirLight.ambient));
      ImGui::ColorEdit3("Dir Diffuse", glm::value_ptr(dirLight.diffuse));
      ImGui::ColorEdit3("Dir Specular", glm::value_ptr(dirLight.specular));

      ImGui::Separator();

      for (int i = 0; i < 4; i++) {
        std::string label = "Point Light " + std::to_string(i);

        if (ImGui::TreeNode(label.c_str())) {
          std::string enabled = "Enabled##" + std::to_string(i);
          ImGui::Checkbox(enabled.c_str(), &pointLights[i].enabled);

          std::string pos = "Position##" + std::to_string(i);
          ImGui::DragFloat3(pos.c_str(),
                            glm::value_ptr(pointLights[i].position), 0.1f);

          std::string ambient = "Ambient##" + std::to_string(i);
          ImGui::ColorEdit3(ambient.c_str(),
                            glm::value_ptr(pointLights[i].ambient));

          std::string diffuse = "Diffuse##" + std::to_string(i);
          ImGui::ColorEdit3(diffuse.c_str(),
                            glm::value_ptr(pointLights[i].diffuse));

          std::string specular = "Specular##" + std::to_string(i);
          ImGui::ColorEdit3(specular.c_str(),
                            glm::value_ptr(pointLights[i].specular));

          std::string constant = "Constant##" + std::to_string(i);
          ImGui::DragFloat(constant.c_str(), &pointLights[i].constant, 0.01f,
                           0.0f, 5.0f);

          std::string linear = "Linear##" + std::to_string(i);
          ImGui::DragFloat(linear.c_str(), &pointLights[i].linear, 0.001f, 0.0f,
                           1.0f);

          std::string quadratic = "Quadratic##" + std::to_string(i);
          ImGui::DragFloat(quadratic.c_str(), &pointLights[i].quadratic, 0.001f,
                           0.0f, 1.0f);

          ImGui::TreePop();
        }
      }

      ImGui::Separator();

      ImGui::Text("Spotlight");

      ImGui::Checkbox("Enable Spotlight", &spotLight.enabled);

      ImGui::ColorEdit3("Spot Ambient", glm::value_ptr(spotLight.ambient));
      ImGui::ColorEdit3("Spot Diffuse", glm::value_ptr(spotLight.diffuse));
      ImGui::ColorEdit3("Spot Specular", glm::value_ptr(spotLight.specular));
      ImGui::DragFloat("Spot Constant", &spotLight.constant, 0.01f);
      ImGui::DragFloat("Spot Linear", &spotLight.linear, 0.001f);
      ImGui::DragFloat("Spot Quadratic", &spotLight.quadratic, 0.001f);
      ImGui::SliderFloat("CutOff", &spotLight.cutOff, 0.0f, 45.0f);
      ImGui::SliderFloat("Outer CutOff", &spotLight.outerCutOff, 0.0f, 45.0f);

      ImGui::Separator();

      ImGui::Text("Material");

      ImGui::SliderFloat("Shininess", &shininess, 1.0f, 256.0f);
      ImGui::ColorEdit3("Emission Color", glm::value_ptr(emissionColor));
      ImGui::SliderFloat("Emission Strength", &emissionStrength, 0.0f, 10.0f);

      ImGui::End();
    }

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

    std::map<float, glm::vec3> sorted;
    for (unsigned int i = 0; i < windows.size(); i++) {
      float distance =
          glm::length(globalApplication::camera.Position - windows[i]);
      sorted[distance] = windows[i];
    }

    glm::vec3 positions[2] = {glm::vec3(-1.0f, 0.0f, -1.0f),
                              glm::vec3(2.0f, 0.0f, 0.0f)};

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST); // enable depth testing for 3D
    glEnable(GL_CULL_FACE);

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = globalApplication::camera.GetViewMatrix();
    glm::mat4 projection = glm::perspective(
        glm::radians(globalApplication::camera.Zoom),
        (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.001f, 100.0f);
    glm::mat4 skyboxView = glm::mat4(glm::mat3(view));

    glBindBuffer(GL_UNIFORM_BUFFER, uboMatricies);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4),
                    glm::value_ptr(projection));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBuffer(GL_UNIFORM_BUFFER, uboMatricies);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4),
                    glm::value_ptr(view));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

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

    lightingShader.use();
    lightingShader.setMat4("projection", projection);
    lightingShader.setMat4("view", view);
    lightingShader.setVec3("viewPos", globalApplication::camera.Position);

    lightCubeShader.use();
    lightCubeShader.setMat4("projection", projection);
    lightCubeShader.setMat4("view", view);

    // draw 4 cubes
    // RED
    glBindVertexArray(regularCubeVAO);
    shaderRed.use();
    model =
        glm::translate(model, glm::vec3(-0.75f, 0.75f, 0.0f)); // move top-left
    shaderRed.setMat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    // GREEN
    shaderGreen.use();
    model = glm::mat4(1.0f);
    model =
        glm::translate(model, glm::vec3(0.75f, 0.75f, 0.0f)); // move top-right
    shaderGreen.setMat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    // YELLOW
    shaderYellow.use();
    model = glm::mat4(1.0f);
    model = glm::translate(model,
                           glm::vec3(-0.75f, -0.75f, 0.0f)); // move bottom-left
    shaderYellow.setMat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    // BLUE
    shaderBlue.use();
    model = glm::mat4(1.0f);
    model = glm::translate(model,
                           glm::vec3(0.75f, -0.75f, 0.0f)); // move bottom-right
    shaderBlue.setMat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    // Regular textured cubes.
    shader.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, cubeTexture);
    glBindVertexArray(cubeVAO);
    for (int i = 0; i < 2; i++) {
      glm::vec3 pos = positions[i];

      model = glm::mat4(1.0f);
      model = glm::translate(model, pos);
      shader.setMat4("model", model);
      glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    glBindVertexArray(0);

    lightingShader.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.02f, 1.5f));
    model = glm::scale(model, glm::vec3(0.3f));
    lightingShader.setMat4("model", model);
    backpack.Draw(lightingShader);

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
    // reverse iterator so we draw the ones behind first before the ones in
    // front
    for (std::map<float, glm::vec3>::reverse_iterator it = sorted.rbegin();
         it != sorted.rend(); ++it) {
      model = glm::mat4(1.0f);
      model = glm::translate(model, it->second);
      windowShader.setMat4("model", model);
      glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    glBindVertexArray(0);
    glDepthMask(GL_TRUE);

    lightingShader.use();

    // Directional light
    lightingShader.setVec3("dirLight.direction", dirLight.direction);

    lightingShader.setVec3("dirLight.ambient", dirLight.enabled
                                                   ? dirLight.ambient
                                                   : glm::vec3(0.0f));
    lightingShader.setVec3("dirLight.diffuse", dirLight.enabled
                                                   ? dirLight.diffuse
                                                   : glm::vec3(0.0f));
    lightingShader.setVec3("dirLight.specular", dirLight.enabled
                                                    ? dirLight.specular
                                                    : glm::vec3(0.0f));

    // Point lights
    for (int i = 0; i < 4; i++) {
      std::string index = "pointLights[" + std::to_string(i) + "]";

      lightingShader.setVec3(index + ".position", pointLights[i].position);
      lightingShader.setVec3(index + ".ambient", pointLights[i].enabled
                                                     ? pointLights[i].ambient
                                                     : glm::vec3(0.0f));
      lightingShader.setVec3(index + ".diffuse", pointLights[i].enabled
                                                     ? pointLights[i].diffuse
                                                     : glm::vec3(0.0f));
      lightingShader.setVec3(index + ".specular", pointLights[i].enabled
                                                      ? pointLights[i].specular
                                                      : glm::vec3(0.0f));
      lightingShader.setFloat(index + ".constant", pointLights[i].constant);
      lightingShader.setFloat(index + ".linear", pointLights[i].linear);
      lightingShader.setFloat(index + ".quadratic", pointLights[i].quadratic);
    }

    // Spotlight
    lightingShader.setVec3("spotLight.position",
                           globalApplication::camera.Position);
    lightingShader.setVec3("spotLight.direction",
                           globalApplication::camera.Front);

    lightingShader.setVec3("spotLight.ambient", spotLight.enabled
                                                    ? spotLight.ambient
                                                    : glm::vec3(0.0f));
    lightingShader.setVec3("spotLight.diffuse", spotLight.enabled
                                                    ? spotLight.diffuse
                                                    : glm::vec3(0.0f));
    lightingShader.setVec3("spotLight.specular", spotLight.enabled
                                                     ? spotLight.specular
                                                     : glm::vec3(0.0f));
    lightingShader.setFloat("spotLight.constant", spotLight.constant);
    lightingShader.setFloat("spotLight.linear", spotLight.linear);
    lightingShader.setFloat("spotLight.quadratic", spotLight.quadratic);
    lightingShader.setFloat("spotLight.cutOff",
                            glm::cos(glm::radians(spotLight.cutOff)));
    lightingShader.setFloat("spotLight.outerCutOff",
                            glm::cos(glm::radians(spotLight.outerCutOff)));

    lightingShader.setFloat("material.shininess", shininess);
    lightingShader.setFloat("material.emissionStrength", emissionStrength);
    lightingShader.setVec3("material.emissionColor", emissionColor);

    glBindVertexArray(lightCubeVAO);
    model = glm::mat4(1.0f);
    lightingShader.setMat4("model", model);

    lightCubeShader.use();

    for (unsigned int i = 0; i < 4; i++) {
      model = glm::mat4(1.0f);
      model = glm::translate(model, pointLights[i].position);
      model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
      lightCubeShader.setMat4("model", model);
      lightCubeShader.setVec3("color", pointLights[i].enabled
                                           ? pointLights[i].diffuse
                                           : glm::vec3(0.0f));
      glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    // Rendering
    // (Your code clears your framebuffer, renders your other stuff etc.)
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

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
