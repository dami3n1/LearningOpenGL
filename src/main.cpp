#include <cmath>
#include <iostream>
#include <iomanip>
#include <ostream>
#include <chrono>

#include "shader_reader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "controller.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "windowSystem.h"
#include "logger.h"
#include "imguiLayer.h"

#include "camera.h"
#include "model.h"

int SCREEN_HEIGHT = 600;
int SCREEN_WIDTH = 800;

// stores how much we're seeing of either texture
float mixValue = 0.2f;
Controller controller;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCREEN_WIDTH / 2.0f;
float lastY = SCREEN_HEIGHT / 2.0f;
bool firstMouse = true;
bool imguitoggle = false;
bool tabwasPressed = false;

// timing
float deltaTime = 0.0f; // time between current frame and last frame
float lastFrame = 0.0f;

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

    bool enabled = true;
};

DirectionalLight dirLight;

PointLight pointLights[4] =
{
    {glm::vec3(0.7f, 0.2f, 2.0f)},
    {glm::vec3(2.3f, -3.3f, -4.0f)},
    {glm::vec3(-4.0f, 2.0f, -12.0f)},
    {glm::vec3(0.0f, 0.0f, -3.0f)}
};

SpotLight spotLight;

glm::vec3 emissionColor = glm::vec3(0.0f, 0.0f, 1.0f);
float emissionStrength = 0.0f;
float shininess = 32.0f;

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xposIn, double yposIn) {
    if (imguitoggle)
        return;

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        mixValue += 0.02f; // change this value accordingly (might be too slow or too fast based on system hardware)
        if (mixValue >= 1.0f)
            mixValue = 1.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        mixValue -= 0.02f; // change this value accordingly (might be too slow or too fast based on system hardware)
        if (mixValue <= 0.0f)
            mixValue = 0.0f;
    }

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    int tabState = glfwGetKey(window, GLFW_KEY_TAB);

    if (tabState == GLFW_PRESS && !tabwasPressed) {
        tabwasPressed = true;

        imguitoggle = !imguitoggle;

        ImGuiIO &io = ImGui::GetIO();

        if (imguitoggle) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

            io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
        } else {
            io.ConfigFlags |= ImGuiConfigFlags_NoMouse;

            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

            firstMouse = true;
        }
    } else if (tabState == GLFW_RELEASE) {
        tabwasPressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);

    Vec2 left = controller.leftStick();

    float speed = 1.5f;
    float deadzone = 0.2f;

    float inputY = left.y;

    if (fabs(inputY) < deadzone)
        inputY = 0.0f;

    mixValue += (-inputY) * speed * deltaTime;

    mixValue = std::clamp(mixValue, 0.0f, 1.0f);
}

int main() {
    if (!windowSystem::glfw_init()) {
        logger(ERROR, "windowSystem::Failed to initialize GLFW");
        return -1; // or stop engine
    }
    GLFWwindow *window = windowSystem::makeWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "OpenGL Window");
    if (!window) {
        logger(ERROR, "windowSystem::Failed to create GLFW window");
        windowSystem::glfw_shutdown();
        return -1;
    }

    // disable vsync for uncapped framerate
    glfwSwapInterval(0);

    glfwSetScrollCallback(window, scroll_callback);

    glfwSetCursorPosCallback(window, mouse_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // initialize GLAD before we can use any opengl functions
    // cast's the glfw function which gives the OS specific function for GLAD to find
    // the OpenGL function pointer (memory address of opengl executable command)(hardware specific)
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        logger(ERROR, "Failed to initialize GLAD");
        return -1;
    }

    glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    if (!imguiLayer::imguiSetup(window)) {
        logger(ERROR, "Failed to initialize ImGui");
        return -1;
    }

    Shader lightingShader("../shaders/colors.vert", "../shaders/colors.frag");
    Shader lightCubeShader("../shaders/light_cube.vert", "../shaders/light_cube.frag");

    float vertices[] = {
        // positions
        -0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f, 0.5f, -0.5f,
        0.5f, 0.5f, -0.5f,
        -0.5f, 0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f, -0.5f, 0.5f,
        0.5f, -0.5f, 0.5f,
        0.5f, 0.5f, 0.5f,
        0.5f, 0.5f, 0.5f,
        -0.5f, 0.5f, 0.5f,
        -0.5f, -0.5f, 0.5f,

        -0.5f, 0.5f, 0.5f,
        -0.5f, 0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, 0.5f,
        -0.5f, 0.5f, 0.5f,

        0.5f, 0.5f, 0.5f,
        0.5f, 0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, 0.5f,
        0.5f, 0.5f, 0.5f,

        -0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, 0.5f,
        0.5f, -0.5f, 0.5f,
        -0.5f, -0.5f, 0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f, 0.5f, -0.5f,
        0.5f, 0.5f, -0.5f,
        0.5f, 0.5f, 0.5f,
        0.5f, 0.5f, 0.5f,
        -0.5f, 0.5f, 0.5f,
        -0.5f, 0.5f, -0.5f
    };

    // first, configure the cube's VAO (and VBO)
    unsigned int VBO, lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(lightCubeVAO);

    // note that we update the lamp's position attribute's stride to reflect the updated buffer data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);

    controller.showControllers();

    float lastFrame = 0.0f;

    // Unbind the VBO (optional, just to avoid accidental changes later)
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    int width = SCREEN_WIDTH, height = SCREEN_HEIGHT;

    glEnable(GL_DEPTH_TEST); // enable depth testing for 3D
    stbi_set_flip_vertically_on_load(true);

    auto t0 = std::chrono::high_resolution_clock::now();
    Model ourModel("../assets/Sponza-master/sponza.obj");
    auto t1 = std::chrono::high_resolution_clock::now();

    std::cout << "Total load: "
            << std::chrono::duration<double>(t1 - t0).count()
            << "s\n";

    t0 = std::chrono::high_resolution_clock::now();
    Model ourModel2("../assets/untitled.obj");
    t1 = std::chrono::high_resolution_clock::now();

    std::cout << "Total load: "
            << std::chrono::duration<double>(t1 - t0).count()
            << "s\n";

    // render loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents(); // processes events received in window and returns a response(if requested)
        // input function called each frame
        processInput(window);

        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwGetFramebufferSize(window, &width, &height);
        SCREEN_WIDTH = (float) width;
        SCREEN_HEIGHT = (float) height;

        imguiLayer::imguiRender();
        ImGui::NewFrame();
        {
            ImGui::Begin("Lighting Editor");

            ImGui::Text("Directional Light");
            ImGui::Checkbox("Enable Dir Light", &dirLight.enabled);
            ImGui::DragFloat3("Dir Direction", glm::value_ptr(dirLight.direction), 0.1f);
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
                    ImGui::DragFloat3(pos.c_str(), glm::value_ptr(pointLights[i].position), 0.1f);

                    std::string ambient = "Ambient##" + std::to_string(i);
                    ImGui::ColorEdit3(ambient.c_str(), glm::value_ptr(pointLights[i].ambient));

                    std::string diffuse = "Diffuse##" + std::to_string(i);
                    ImGui::ColorEdit3(diffuse.c_str(), glm::value_ptr(pointLights[i].diffuse));

                    std::string specular = "Specular##" + std::to_string(i);
                    ImGui::ColorEdit3(specular.c_str(), glm::value_ptr(pointLights[i].specular));

                    std::string constant = "Constant##" + std::to_string(i);
                    ImGui::DragFloat(constant.c_str(), &pointLights[i].constant, 0.01f, 0.0f, 5.0f);

                    std::string linear = "Linear##" + std::to_string(i);
                    ImGui::DragFloat(linear.c_str(), &pointLights[i].linear, 0.001f, 0.0f, 1.0f);

                    std::string quadratic = "Quadratic##" + std::to_string(i);
                    ImGui::DragFloat(quadratic.c_str(), &pointLights[i].quadratic, 0.001f, 0.0f, 1.0f);

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

        controller.update();

        glClearColor(0.f, 0.f, 0.f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // be sure to activate shader when setting uniforms/drawing objects
        lightingShader.use();
        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float) SCREEN_WIDTH / (float) SCREEN_HEIGHT,
                                                0.01f, 200.0f);
        glm::mat4 view = camera.GetViewMatrix();
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);

        // render the loaded model
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.015625f, 0.015625f, 0.015625f));
        // it's a bit too big for our scene, so scale it down
        lightingShader.setMat4("model", model);
        ourModel.Draw(lightingShader);

        // render the loaded model
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(1.f, 1.f, 1.f)); // it's a bit too big for our scene, so scale it down
        lightingShader.setMat4("model", model);
        ourModel2.Draw(lightingShader);

        // Directional light
        lightingShader.setVec3("dirLight.direction", dirLight.direction);
        lightingShader.setVec3("dirLight.ambient", dirLight.enabled ? dirLight.ambient : glm::vec3(0.0f));
        lightingShader.setVec3("dirLight.diffuse", dirLight.enabled ? dirLight.diffuse : glm::vec3(0.0f));
        lightingShader.setVec3("dirLight.specular", dirLight.enabled ? dirLight.specular : glm::vec3(0.0f));

        //use lightcube shader for the cube light
        lightCubeShader.use();
        lightCubeShader.setMat4("projection", projection);
        lightCubeShader.setMat4("view", view);
        glBindVertexArray(lightCubeVAO);
        model = glm::mat4(1.0f);
        lightingShader.setMat4("model", model);
        // Point lights
        for (int i = 0; i < 4; i++) {
            lightCubeShader.use();
            model = glm::mat4(1.0f);
            model = glm::translate(model, pointLights[i].position);
            model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
            lightCubeShader.setVec3("color", pointLights[i].enabled ? pointLights[i].diffuse : glm::vec3(0.0f));
            lightCubeShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);

            //switch back to using the lighting shader
            lightingShader.use();
            std::string index = "pointLights[" + std::to_string(i) + "]";
            lightingShader.setVec3(index + ".position", pointLights[i].position);
            lightingShader.setVec3(index + ".ambient",
                                   pointLights[i].enabled ? pointLights[i].ambient : glm::vec3(0.0f));
            lightingShader.setVec3(index + ".diffuse",
                                   pointLights[i].enabled ? pointLights[i].diffuse : glm::vec3(0.0f));
            lightingShader.setVec3(index + ".specular",
                                   pointLights[i].enabled ? pointLights[i].specular : glm::vec3(0.0f));
            lightingShader.setFloat(index + ".constant", pointLights[i].constant);
            lightingShader.setFloat(index + ".linear", pointLights[i].linear);
            lightingShader.setFloat(index + ".quadratic", pointLights[i].quadratic);
        }

        // Spotlight
        lightingShader.setVec3("spotLight.position", camera.Position);
        lightingShader.setVec3("spotLight.direction", camera.Front);
        lightingShader.setVec3("spotLight.ambient", spotLight.enabled ? spotLight.ambient : glm::vec3(0.0f));
        lightingShader.setVec3("spotLight.diffuse", spotLight.enabled ? spotLight.diffuse : glm::vec3(0.0f));
        lightingShader.setVec3("spotLight.specular", spotLight.enabled ? spotLight.specular : glm::vec3(0.0f));
        lightingShader.setFloat("spotLight.constant", spotLight.constant);
        lightingShader.setFloat("spotLight.linear", spotLight.linear);
        lightingShader.setFloat("spotLight.quadratic", spotLight.quadratic);
        lightingShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(spotLight.cutOff)));
        lightingShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(spotLight.outerCutOff)));
        lightingShader.setVec3("viewPos", camera.Position);

        //material
        lightingShader.setFloat("material.shininess", shininess);
        lightingShader.setFloat("material.emissionStrength", emissionStrength);
        lightingShader.setVec3("material.emissionColor", emissionColor);

        // Rendering
        // (Your code clears your framebuffer, renders your other stuff etc.)
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        // (Your code calls glfwSwapBuffers() etc.)

        glfwSwapBuffers(window); // swaps color buffer in window
    }

    imguiLayer::Shutdown();

    // de allocated resources after usage
    glDeleteVertexArrays(1, &lightCubeVAO);
    glDeleteBuffers(1, &VBO);

    windowSystem::glfw_shutdown();

    return 0;
}
