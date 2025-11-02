#pragma once
#include "imgui.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class DebugWindow {
public:
    bool enabled = false;

    void init(GLFWwindow* window);
    void shutdown();
    void newFrame();
    void render(float deltaTime, float& cameraSpeed, int& renderDistance, bool& vsyncEnabled, ImVec4& clearColor, glm::vec3& cameraPos, glm::vec3& cameraFront);
    void renderImGui();
    void updateFps(float newDeltaTime);

private:
    static const int FPS_HISTORY_SIZE = 60;
    float fpsValues[FPS_HISTORY_SIZE] = {0};
    int fpsIndex = 0;
    int frameCount = 0;
    float lastFPSUpdate = 0.0f;
};
