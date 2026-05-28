#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/postprocess.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <format>
#include <unordered_map>

#include "engine/Shader.h"
#include "engine/Texture.h"
#include "engine/Scheduler.h"
#include "world/World.h"
#include "engine/DebugWindow.h"
#include "engine/Ray.h"
#include "engine/Camera.h"
#include "imgui.h"
#include "engine/Input.h"
#include "engine/MeshFactory.h"
#include "engine/Model.h"
#include "engine/Time.h"

Time gameTime;

bool wireframe = false;
Camera camera;
int renderDistance = 8;
uint8_t hotbarSlot = 1;
float fovTarget = 15.0;;
bool vsyncEnabled = true;
glm::vec3 clearColor(0.45f, 0.55f, 0.60f);
glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

struct Object {
    Mesh* mesh = nullptr;
    Model* model = nullptr;
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);
};

std::vector<Object> sceneObjects;

void toggleCursorLock(GLFWwindow *window) {
    camera.cursorLock = !camera.cursorLock;
    if (camera.cursorLock) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        camera.firstMouse = true;
        ImGuiIO& io = ImGui::GetIO();
        io.MouseDrawCursor = false;
    } else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        ImGuiIO& io = ImGui::GetIO();
        io.MouseDrawCursor = true;
    }
}

void processInput(GLFWwindow *window, Shader& shader, World& world, DebugWindow& debugWindow) {
    Input::updateKeys(window);

    if (Input::keyJustPressed(GLFW_KEY_ESCAPE))
        toggleCursorLock(window);

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

    static float startTime = 0.0f;

    if (Input::keyJustPressed(GLFW_KEY_C))
        startTime = gameTime.getTotalTime();

    if (Input::keyPressed(GLFW_KEY_C)) {
        float t = (gameTime.getTotalTime() - startTime) / 0.5f;
        t = t > 1 ? 1 : t;        // clamp
        t = t*t*(3-2*t);          // smoothstep
        camera.fov = 45.0f + t * (fovTarget - 45);
    }
    if (Input::keyJustReleased(GLFW_KEY_C))
        camera.fov = 45.0f;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (action == GLFW_PRESS) {
        auto* world = static_cast<World*>(glfwGetWindowUserPointer(window));
        const Ray ray(camera.pos, camera.front);
        if (auto raycastResult = ray.cast(*world, 10.0f)) {
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

void mouse_callback(GLFWwindow* window, const double xPosIn, const double yPosIn) {
    camera.mouseLook(xPosIn, yPosIn);
}

void scroll_callback(GLFWwindow* window, double xOffset, double yOffset) {
    if (Input::keyPressed(GLFW_KEY_C)) {
        fovTarget = static_cast<float>(std::clamp(fovTarget - yOffset * 1.0, 1.0, 40.0));
    } else {
        hotbarSlot = (hotbarSlot + static_cast<int>(yOffset) + 9) % 9;
    }
}

unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 600;

void framebuffer_size_callback(GLFWwindow* window, const int width, const int height) {
    glViewport(0, 0, width, height);
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
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
    Shader modelShader("../src/shader/model.vert", "../src/shader/model.frag");
    Texture texture("../assets/atlas.png");
    shader.setInt("texture1", 0);
    Shader selectionShader("../src/shader/selection.vert", "../src/shader/selection.frag");

    Model backpack("../assets/models/backpack/backpack.obj", aiProcess_FlipUVs);
    Model monkey("../assets/models/monkey/monkey.obj");
    Model tank("../assets/models/tiger/Tiger_I.obj");
    Mesh plane = MeshFactory::plane(5,5);
    Mesh cube = MeshFactory::cube(1);
    Mesh sphere = MeshFactory::sphere(5,20);

    sceneObjects.push_back({ &plane, nullptr, glm::vec3(-15.5f, 135.0f, 5.5f) });
    sceneObjects.push_back({ &cube,  nullptr, glm::vec3(-10.5f, 135.0f, 15.5f) });
    sceneObjects.push_back({ &sphere,nullptr, glm::vec3(-20.5f, 135.0f, -20.5f) });
    sceneObjects.push_back({ nullptr, &backpack, glm::vec3(16.5f, 135.0f, 0.5f) });
    sceneObjects.push_back({ nullptr, &monkey,   glm::vec3(10.5f, 135.0f, 0.5f) });
    sceneObjects.push_back({ nullptr, &tank,     glm::vec3(-10.5f, 135.0f, 0.5f) });


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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    DebugWindow debugWindow;
    DebugWindow::init(window);

    Scheduler scheduler;

    while (!glfwWindowShouldClose(window)) {
        gameTime.update();

        debugWindow.updateFps();
        scheduler.update();

        processInput(window, shader, world, debugWindow);
        world.update(camera.pos, renderDistance);
        camera.creativeMovement();

        glClearColor(clearColor.r, clearColor.g, clearColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        texture.bind();

        float radius = 300.0f;
        float worldTime = gameTime.getTotalTime() / 5.0f;
        glm::vec3 sunPos(radius * std::cos(worldTime), 100.0f, radius * std::sin(worldTime));

        glm::mat4 view = glm::lookAt(camera.pos, camera.pos + camera.front, camera.up);
        glm::mat4 projection = glm::perspective(glm::radians(camera.fov), static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT), 0.1f, 1000.0f);
        shader.use();
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);
        shader.setVec3("viewPos", camera.pos);
        shader.setVec3("lightColor", lightColor);
        shader.setVec3("lightPos", sunPos);

        world.render(shader);

        modelShader.use();
        modelShader.setMat4("view", view);
        modelShader.setMat4("projection", projection);
        modelShader.setVec3("viewPos", camera.pos);
        modelShader.setVec3("lightColor", lightColor);
        modelShader.setVec3("lightPos", sunPos);

        for (auto& obj : sceneObjects) {
            glm::mat4 model = glm::translate(glm::mat4(1.0f), obj.position);
            model = glm::rotate(model, glm::radians(obj.rotation.x), glm::vec3(1,0,0));
            model = glm::rotate(model, glm::radians(obj.rotation.y), glm::vec3(0,1,0));
            model = glm::rotate(model, glm::radians(obj.rotation.z), glm::vec3(0,0,1));
            model = glm::scale(model, obj.scale);

            modelShader.setMat4("model", model);

            if (obj.mesh) {
                modelShader.setBool("hasTexture", !obj.mesh->textures.empty());
                obj.mesh->draw(modelShader);
            } else if (obj.model) {
                modelShader.setBool("hasTexture", true);
                obj.model->draw(modelShader);
            }
        }


        Ray ray(camera.pos, camera.front);
        if (auto raycastResult = ray.cast(world, 10.0f)) {
            selectionShader.use();
            selectionShader.setMat4("view", view);
            selectionShader.setMat4("projection", projection);
            auto model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(raycastResult->blockPos) + glm::vec3(0.5f));
            model = glm::scale(model, glm::vec3(1.01f));
            selectionShader.setMat4("model", model);

            glBindVertexArray(selectionVAO);
            glLineWidth(3.0f);
            glDrawArrays(GL_LINES, 0, 24);
            glLineWidth(1.0f);
        }


        DebugWindow::newFrame();
        debugWindow.render(camera.speed, renderDistance, vsyncEnabled, clearColor, camera.pos, camera.front, lightColor);
        ImGui::GetBackgroundDrawList()->AddText(
            ImVec2(20, 20),
            IM_COL32(255, 255, 255, 255),
            std::to_string(hotbarSlot + 1).c_str()
        );
        DebugWindow::renderImGui();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    DebugWindow::shutdown();

    glfwTerminate();
    return 0;
}
