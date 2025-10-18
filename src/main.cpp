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
#include "Shader.h"
#include "stb_image.h"

float deltaTime = 0.0f;
float lastFrame = 0.0f;

bool wireframe = false;
glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f,  3.0f);
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

void processInput(GLFWwindow *window, Shader& shader) {
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

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSwapInterval(vsyncEnabled ? 1 : 0);

    glewInit();

    glEnable(GL_DEPTH_TEST);

    Shader shader("src/shader/vert.glsl", "src/shader/frag.glsl");
    unsigned int texture = loadTexture("assets/brick.jpg");
    shader.setInt("texture1", texture);

    float vertices[] = {
        // positions          // tex coords (repeat per face)
        -0.5f,-0.5f,-0.5f, 0,0,  0.5f,-0.5f,-0.5f, 1,0,  0.5f,0.5f,-0.5f, 1,1,  -0.5f,0.5f,-0.5f,0,1,  // back
        -0.5f,-0.5f, 0.5f, 0,0,  0.5f,-0.5f, 0.5f, 1,0,  0.5f,0.5f, 0.5f, 1,1,  -0.5f,0.5f, 0.5f,0,1,  // front
        -0.5f,-0.5f,-0.5f,0,0, -0.5f,0.5f,-0.5f,1,0, -0.5f,0.5f,0.5f,1,1, -0.5f,-0.5f,0.5f,0,1,         // left
        0.5f,-0.5f,-0.5f,0,0,  0.5f,0.5f,-0.5f,1,0,  0.5f,0.5f,0.5f,1,1,  0.5f,-0.5f,0.5f,0,1,         // right
        -0.5f,-0.5f,-0.5f,0,0,  0.5f,-0.5f,-0.5f,1,0,  0.5f,-0.5f,0.5f,1,1,  -0.5f,-0.5f,0.5f,0,1,       // bottom
        -0.5f,0.5f,-0.5f,0,0,   0.5f,0.5f,-0.5f,1,0,   0.5f,0.5f,0.5f,1,1,   -0.5f,0.5f,0.5f,0,1        // top
    };

    unsigned int indices[36];
    for(int i=0;i<6;i++){
        indices[i*6+0]=i*4+0; indices[i*6+1]=i*4+1; indices[i*6+2]=i*4+2;
        indices[i*6+3]=i*4+2; indices[i*6+4]=i*4+3; indices[i*6+5]=i*4+0;
    }

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    // remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0); 

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window, shader);
        
        float w = clear_color.w;
        glClearColor(clear_color.x, clear_color.y, clear_color.z, w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplGlfw_NewFrame();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        
        shader.use();

        
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        
        glm::mat4 projection;
        projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
        
        for (int x = 0; x < 10; x++) {
            for (int z = 0; z < 10; z++) {
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(x, -5.0f, z));
                //float angle = 0.0f * i;
                //model = glm::rotate(model, glm::radians(angle), glm::vec3(0.7f, 0.5f, 0.3f));
                shader.setMat4("model", model);
                
                glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
            }
        }
        
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);
        
        glBindVertexArray(VAO);

        if (debugWindow) {
        ImGui::Begin("Debug Window", nullptr, ImGuiWindowFlags_NoTitleBar);

        if (ImGui::BeginTabBar("TabBar", ImGuiTabBarFlags_None)) {
            if (ImGui::BeginTabItem("General")) {
                ImGui::Text("Sky Color:");
                ImGui::ColorEdit3("", (float*)&clear_color);
                ImGui::Text("Speed");
                ImGui::SliderFloat("Speed", &cameraSpeed, 0, 100);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Debugger")) {
                if (ImGui::Checkbox("VSync", &vsyncEnabled)) {
                    glfwSwapInterval(vsyncEnabled ? 1 : 0);
                }
                ImGui::Text(std::format("DeltaTime: {:.6f}", deltaTime).c_str());
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

        float lastFrame = 0.0f;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    
    glfwTerminate();
    return 0;
}
