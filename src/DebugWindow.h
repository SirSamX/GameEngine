#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class DebugWindow {
public:
    bool enabled = false;

    static void init(GLFWwindow* window);
    static void shutdown();
    static void newFrame();
    void render(float deltaTime, float& cameraSpeed, int& renderDistance, bool& vsyncEnabled, glm::vec3& clearColor, glm::vec3& cameraPos, glm::vec3& cameraFront, glm::vec3& lightColor);
    static void renderImGui();
    void updateFps(float newDeltaTime);

private:
    static constexpr int FPS_HISTORY_SIZE = 60;
    float fpsValues[FPS_HISTORY_SIZE] = {0};
    int fpsIndex = 0;
    int frameCount = 0;
    float lastFPSUpdate = 0.0f;
};
