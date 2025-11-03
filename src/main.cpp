#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <format>
#include <unordered_map>
#include <filesystem>

#include "Shader.h"
#include "Texture.h"
#include "Scheduler.h"
#include "World.h"
#include "Block.h"
#include "DebugWindow.h"
#include "Ray.h"
#include "Camera.h"
#include "Input.h"
#include "Model.h"

float deltaTime = 0.0f;
float lastFrame = 0.0f;

bool wireframe = false;
Camera camera;
int renderDistance = 8;
uint8_t hotbarSlot = 1;
float fovTarget = 15.0;;
bool vsyncEnabled = true;
glm::vec3 clearColor(0.45f, 0.55f, 0.60f);
glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

void processInput(GLFWwindow *window, Shader& shader, World& world, DebugWindow& debugWindow) {
    Input::updateKeys(window);

    // Exit
    if (Input::keyJustPressed(GLFW_KEY_ESCAPE)) {
        camera.cursorLock = !camera.cursorLock;
        if (camera.cursorLock) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }

    // Debug Keys
    if (Input::keyJustPressed(GLFW_KEY_X))
        glfwSetWindowShouldClose(window, true);
    if (Input::keyJustPressed(GLFW_KEY_F1)) {
        wireframe = !wireframe;
        if (wireframe)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    if (Input::keyJustPressed(GLFW_KEY_F2))
        shader.reload();
    if (Input::keyJustPressed(GLFW_KEY_F3))
        debugWindow.enabled = !debugWindow.enabled;
    if (Input::keyJustPressed(GLFW_KEY_R))
        world.chunks.clear();

    static double startTime = 0;

    if (Input::keyJustPressed(GLFW_KEY_C))
        startTime = glfwGetTime();

    if (Input::keyPressed(GLFW_KEY_C)) {
        double t = (glfwGetTime() - startTime) / 0.5;
        t = t > 1 ? 1 : t;        // clamp
        t = t*t*(3-2*t);          // smoothstep
        camera.fov = 45 + t * (fovTarget - 45);
    }
    if (Input::keyJustReleased(GLFW_KEY_C))
        camera.fov = 45.0f;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (action == GLFW_PRESS) {
        World* world = static_cast<World*>(glfwGetWindowUserPointer(window));
        Ray ray(camera.pos, camera.front);
        auto raycastResult = ray.cast(*world, 10.0f);
        if (raycastResult) {
            if (button == GLFW_MOUSE_BUTTON_LEFT) {
                world->setBlock(raycastResult->blockPos, 0);
            }
            else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
                if (hotbarSlot < 1 || hotbarSlot > 2) return;
                world->setBlock(raycastResult->blockPos + raycastResult->face, hotbarSlot);
            }
        }
    }
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    camera.mouseLook(xposIn, yposIn);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    if (Input::keyPressed(GLFW_KEY_C)) {
        fovTarget = std::clamp(fovTarget - yoffset * 1.0, 1.0, 40.0);
    } else {
        hotbarSlot = (hotbarSlot + static_cast<int>(yoffset) - 1 + 9) % 9 + 1;
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

int main() {
    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Test", nullptr, nullptr);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);

    World world;
    glfwSetWindowUserPointer(window, &world);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSwapInterval(vsyncEnabled ? 1 : 0);

    glewInit();

    glEnable(GL_DEPTH_TEST);

    Shader shader("../src/shader/default.vert", "../src/shader/default.frag");
    shader.setInt("texture1", 0);
    Shader selectionShader("../src/shader/selection.vert", "../src/shader/selection.frag");
    Texture texture("../assets/atlas.png");
    Model backpack("../assets/models/backpack/backpack.obj");

    float vertices[] = {
        // Front face
        -0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,  0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f, -0.5f, -0.5f,  0.5f,
        // Back face
        -0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,  0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f, -0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f, -0.5f, -0.5f, -0.5f,
        // Connecting lines
        -0.5f, -0.5f,  0.5f, -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,  0.5f, -0.5f, -0.5f,
        -0.5f,  0.5f,  0.5f, -0.5f,  0.5f, -0.5f,
         0.5f,  0.5f,  0.5f,  0.5f,  0.5f, -0.5f
    };
    unsigned int selectionVAO, selectionVBO;
    glGenVertexArrays(1, &selectionVAO);
    glGenBuffers(1, &selectionVBO);
    glBindVertexArray(selectionVAO);
    glBindBuffer(GL_ARRAY_BUFFER, selectionVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    DebugWindow debugWindow;
    debugWindow.init(window);
    
    Scheduler scheduler;

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        debugWindow.updateFps(deltaTime);
        scheduler.update();

        processInput(window, shader, world, debugWindow);
        world.update(camera.pos, renderDistance);
        camera.creativeMovement(deltaTime);

        glClearColor(clearColor.r, clearColor.g, clearColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
        
        texture.bind();
                
        glm::mat4 view = glm::lookAt(camera.pos, camera.pos + camera.front, camera.up);
        glm::mat4 projection = glm::perspective(glm::radians(camera.fov), 800.0f / 600.0f, 0.1f, 1000.0f);
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);
        shader.setVec3("viewPos", camera.pos);
        shader.setVec3("lightColor", lightColor);

        /*float radius = 300.0f;
        float time = glfwGetTime() / 5;
        glm::vec3 sunPos(radius * cos(time), 100.0f, radius * sin(time));*/
        shader.setVec3("lightPos", 0, 200, 0);

        world.render(shader);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(16.5f, 135.0f, 0.5f));
        shader.setMat4("model", model);
        backpack.draw(shader);

        Ray ray(camera.pos, camera.front);
        auto raycastResult = ray.cast(world, 10.0f);
        if (raycastResult) {
            selectionShader.use();
            selectionShader.setMat4("view", view);
            selectionShader.setMat4("projection", projection);
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(raycastResult->blockPos) + glm::vec3(0.5f));
            model = glm::scale(model, glm::vec3(1.01f));
            selectionShader.setMat4("model", model);

            glBindVertexArray(selectionVAO);
            glLineWidth(3.0f);
            glDrawArrays(GL_LINES, 0, 24);
            glLineWidth(1.0f);
        }

        
        debugWindow.newFrame();
        debugWindow.render(deltaTime, camera.speed, renderDistance, vsyncEnabled, clearColor, camera.pos, camera.front, lightColor);
        ImGui::GetBackgroundDrawList()->AddText(
            ImVec2(20, 20),
            IM_COL32(255, 255, 255, 255),
            std::to_string(hotbarSlot).c_str()
        );
        debugWindow.renderImGui();
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    debugWindow.shutdown();
    
    glfwTerminate();
    return 0;
}
