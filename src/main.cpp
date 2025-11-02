#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <format>
#include <unordered_map>
#include <algorithm>

#include "Shader.h"
#include "Texture.h"
#include "Scheduler.h"
#include "World.h"
#include "Block.h"
#include "DebugWindow.h"
#include "Ray.h"

float deltaTime = 0.0f;
float lastFrame = 0.0f;

bool wireframe = false;
glm::vec3 cameraPos   = glm::vec3(16.5f, 130.0f, 0.5f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);

bool cursorLock = true;
bool firstMouse = true;
float yaw   = -90.0f;
float pitch =  0.0f;
float lastX =  800.0f / 2.0;
float lastY =  600.0 / 2.0;
float fov   =  45.0f;
float cameraSpeed = 15.0f;
int renderDistance = 4;

bool vsyncEnabled = true;
glm::vec3 clearColor = glm::vec3(0.45f, 0.55f, 0.60f);
glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);

struct KeyState {
    bool pressed = false;
    bool justPressed = false;
    bool justReleased = false;
};

std::unordered_map<int, KeyState> keys;

void updateKeys(GLFWwindow* window) {
    for (auto& [key, state] : keys) {
        bool isPressedNow = glfwGetKey(window, key) == GLFW_PRESS;
        state.justPressed = isPressedNow && !state.pressed;
        state.justReleased = !isPressedNow && state.pressed;
        state.pressed = isPressedNow;
    }
}

bool keyJustPressed(int key) {
    return keys[key].justPressed;
}

bool keyJustReleased(int key) {
    return keys[key].justReleased;
}

bool keyPressed(int key) {
    return keys[key].pressed;
}

void processInput(GLFWwindow *window, Shader& shader, World& world, DebugWindow& debugWindow) {
    updateKeys(window);

    // Exit
    if (keyJustPressed(GLFW_KEY_ESCAPE)) {
        cursorLock = !cursorLock;
        if (cursorLock) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }

    // Debug Keys
    if (keyJustPressed(GLFW_KEY_X))
        glfwSetWindowShouldClose(window, true);
    if (keyJustPressed(GLFW_KEY_F1)) {
        wireframe = !wireframe;
        if (wireframe)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    if (keyJustPressed(GLFW_KEY_F2))
        shader.reload();
    if (keyJustPressed(GLFW_KEY_F3))
        debugWindow.enabled = !debugWindow.enabled;
    if (keyJustPressed(GLFW_KEY_R)) {
        int chunkX = static_cast<int>(cameraPos.x) / Chunk::WIDTH;
        int chunkZ = static_cast<int>(cameraPos.z) / Chunk::DEPTH;
        world.markChunkDirty(chunkX, chunkZ);
    }

    // Camera movement
    float camSpeed = cameraSpeed * deltaTime;
    if (keyPressed(GLFW_KEY_W))
        cameraPos += camSpeed * cameraFront;
    if (keyPressed(GLFW_KEY_S))
        cameraPos -= camSpeed * cameraFront;
    if (keyPressed(GLFW_KEY_A))
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * camSpeed;
    if (keyPressed(GLFW_KEY_D))
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * camSpeed;
    if (keyPressed(GLFW_KEY_SPACE))
        cameraPos += camSpeed * cameraUp;
    if (keyPressed(GLFW_KEY_LEFT_SHIFT))
        cameraPos -= camSpeed * cameraUp;

    if (keyPressed(GLFW_KEY_C))
        fov = 15.0f;
    if (keyJustReleased(GLFW_KEY_C))
        fov = 45.0f;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (action == GLFW_PRESS) {
        World* world = static_cast<World*>(glfwGetWindowUserPointer(window));
        Ray ray(cameraPos, cameraFront);
        auto raycastResult = ray.cast(*world, 10.0f);
        if (raycastResult) {
            if (button == GLFW_MOUSE_BUTTON_LEFT) {
                world->setBlock(raycastResult->blockPos, 0);
            }
            else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
                world->setBlock(raycastResult->blockPos + raycastResult->face, (uint8_t)Block::Grass);
            }
        }
    }
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    if (!cursorLock) return;

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
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

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {

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

    Shader shader("shader/default.vert", "shader/default.frag");
    shader.setInt("texture1", 0);
    Shader selectionShader("shader/selection.vert", "shader/selection.frag");
    Texture texture("assets/atlas.png");

    float vertices[] = {
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,

        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,

         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,

        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
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
        world.update(cameraPos, renderDistance);

        glClearColor(clearColor.r, clearColor.g, clearColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
        
        texture.bind();
                
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 projection = glm::perspective(glm::radians(fov), 800.0f / 600.0f, 0.1f, 1000.0f);
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);
        shader.setVec3("viewPos", cameraPos);
        shader.setVec3("lightColor", lightColor);

        float radius = 300.0f;
        float time = glfwGetTime() / 5;
        glm::vec3 sunPos(radius * cos(time), 100.0f, radius * sin(time));
        shader.setVec3("lightPos", 0, 200, 0);

        world.render(shader);

        Ray ray(cameraPos, cameraFront);
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
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        
        debugWindow.newFrame();
        debugWindow.render(deltaTime, cameraSpeed, renderDistance, vsyncEnabled, clearColor, cameraPos, cameraFront, lightColor);
        debugWindow.renderImGui();
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    debugWindow.shutdown();
    
    glfwTerminate();
    return 0;
}
