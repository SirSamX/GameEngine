#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <iostream>
#include <format>
#include <unordered_map>
#include <algorithm>
#include "Shader.h"
#include "stb_image.h"
#include "Scheduler.h"
#include "World.h"

float deltaTime = 0.0f;
float lastFrame = 0.0f;

bool wireframe = false;
glm::vec3 cameraPos   = glm::vec3(8.0f, 80.0f, 8.0f);
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

bool debugWindow = false;
bool vsyncEnabled = true;

struct KeyState {
    bool pressed = false;
    bool justPressed = false;
};

std::unordered_map<int, KeyState> keys;

void updateKeys(GLFWwindow* window) {
    for (auto& [key, state] : keys) {
        bool isPressedNow = glfwGetKey(window, key) == GLFW_PRESS;
        state.justPressed = isPressedNow && !state.pressed;
        state.pressed = isPressedNow;
    }
}

bool keyJustPressed(int key) {
    return keys[key].justPressed;
}

bool keyPressed(int key) {
    return keys[key].pressed;
}

void processInput(GLFWwindow *window, Shader& shader, World& world) {
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
        debugWindow = !debugWindow;

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
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (action == GLFW_PRESS) {
        World* world = static_cast<World*>(glfwGetWindowUserPointer(window));
        auto raycastResult = world->raycast(cameraPos, cameraFront, 10.0f);
        if (raycastResult) {
            if (button == GLFW_MOUSE_BUTTON_LEFT) {
                world->setBlock(raycastResult->blockPos, 0);
            }
            else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
                world->setBlock(raycastResult->blockPos + raycastResult->face, 1);
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
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f; // change this value to your liking
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
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
    fov -= (float)yoffset;
    if (fov < 1.0f)
        fov = 1.0f;
    if (fov > 45.0f)
        fov = 45.0f;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

unsigned int loadTexture(const char* path) {
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Failed to load texture: " << path << std::endl;
    }
    stbi_image_free(data);
    return texture;
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

    Shader shader("src/shader/vert.glsl", "src/shader/frag.glsl");
    unsigned int texture = loadTexture("assets/brick.jpg");
    shader.setInt("texture1", texture);
    Shader selectionShader("src/shader/selection.vert", "src/shader/selection.frag");

    float vertices[] = {
        // positions         
        -0.005f, -0.005f, -0.005f,
         0.005f, -0.005f, -0.005f,
         0.005f,  0.005f, -0.005f,
         0.005f,  0.005f, -0.005f,
        -0.005f,  0.005f, -0.005f,
        -0.005f, -0.005f, -0.005f,

        -0.005f, -0.005f,  0.005f,
         0.005f, -0.005f,  0.005f,
         0.005f,  0.005f,  0.005f,
         0.005f,  0.005f,  0.005f,
        -0.005f,  0.005f,  0.005f,
        -0.005f, -0.005f,  0.005f,

        -0.005f,  0.005f,  0.005f,
        -0.005f,  0.005f, -0.005f,
        -0.005f, -0.005f, -0.005f,
        -0.005f, -0.005f, -0.005f,
        -0.005f, -0.005f,  0.005f,
        -0.005f,  0.005f,  0.005f,

         0.005f,  0.005f,  0.005f,
         0.005f,  0.005f, -0.005f,
         0.005f, -0.005f, -0.005f,
         0.005f, -0.005f, -0.005f,
         0.005f, -0.005f,  0.005f,
         0.005f,  0.005f,  0.005f,

        -0.005f, -0.005f, -0.005f,
         0.005f, -0.005f, -0.005f,
         0.005f, -0.005f,  0.005f,
         0.005f, -0.005f,  0.005f,
        -0.005f, -0.005f,  0.005f,
        -0.005f, -0.005f, -0.005f,

        -0.005f,  0.005f, -0.005f,
         0.005f,  0.005f, -0.005f,
         0.005f,  0.005f,  0.005f,
         0.005f,  0.005f,  0.005f,
        -0.005f,  0.005f,  0.005f,
        -0.005f,  0.005f, -0.005f,
    };
    unsigned int selectionVAO, selectionVBO;
    glGenVertexArrays(1, &selectionVAO);
    glGenBuffers(1, &selectionVBO);
    glBindVertexArray(selectionVAO);
    glBindBuffer(GL_ARRAY_BUFFER, selectionVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    
    const int FPS_HISTORY = 60;
    float fpsValues[FPS_HISTORY] = {0};
    int fpsIndex = 0;
    int frameCount = 0;
    float lastFPSUpdate = 0.0f;
    ImVec4 clearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    Scheduler scheduler;

    scheduler.addTask(0.2f, [&]() {
        float fps = frameCount / (static_cast<float>(glfwGetTime()) - lastFPSUpdate);
        fpsValues[fpsIndex] = fps;
        fpsIndex = (fpsIndex + 1) % FPS_HISTORY;

        lastFPSUpdate = static_cast<float>(glfwGetTime());
        frameCount = 0;
    });

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        frameCount++;
        scheduler.update();

        processInput(window, shader, world);
        world.update(cameraPos, renderDistance);

        glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();

        ImGui_ImplGlfw_NewFrame();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 projection = glm::perspective(glm::radians(fov), 800.0f / 600.0f, 0.1f, 1000.0f);
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);

        world.render(shader);

        auto raycastResult = world.raycast(cameraPos, cameraFront, 10.0f);
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

        
        if (debugWindow) {
            ImGui::Begin("Debug Window", nullptr, ImGuiWindowFlags_NoTitleBar);

            if (ImGui::BeginTabBar("TabBar", ImGuiTabBarFlags_None)) {
                if (ImGui::BeginTabItem("General")) {
                    ImGui::Text("Sky Color:");
                    ImGui::ColorEdit3("##skyColor", (float*)&clearColor);
                    ImGui::Text("Speed");
                    ImGui::SliderFloat("Speed", &cameraSpeed, 0, 100);
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Debugger")) {
                    if (ImGui::Checkbox("VSync", &vsyncEnabled))
                        glfwSwapInterval(vsyncEnabled ? 1 : 0);

                    ImGui::Text(std::format("DeltaTime: {:.6f}", deltaTime).c_str());
                    ImGui::Text(std::format("FPS: {:.1f}", fpsValues[fpsIndex]).c_str());
                    float sum = 0.0f;
                    for (float v : fpsValues) sum += v;
                    char overlay[32];
                    sprintf(overlay, "avg %.1f", sum / FPS_HISTORY);

                    auto [minIt, maxIt] = std::minmax_element(fpsValues, fpsValues + FPS_HISTORY);
                    ImGui::PlotLines("##fpsPlot", fpsValues, FPS_HISTORY, fpsIndex, overlay, *minIt * 0.95f, *maxIt * 1.05f, ImVec2(0, 80.0f));

                    ImGui::SliderInt("Render Distance", &renderDistance, 1, 100);

                    //ImGui::Text("Indices: %d", allIndices.size());
                    //ImGui::Text("Vertices: %d", allVertices.size());

                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Stats")) {
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            ImGui::End();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    glfwTerminate();
    return 0;
}
