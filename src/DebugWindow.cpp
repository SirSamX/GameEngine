#include "DebugWindow.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <algorithm>
#include <format>
#include <cstdio>
#include <GLFW/glfw3.h>

void DebugWindow::init(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void DebugWindow::shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void DebugWindow::newFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void DebugWindow::updateFps(float newDeltaTime) {
    frameCount++;
    if (glfwGetTime() - lastFPSUpdate >= 0.5f) {
        float fps = frameCount / (glfwGetTime() - lastFPSUpdate);
        fpsValues[fpsIndex] = fps;
        fpsIndex = (fpsIndex + 1) % FPS_HISTORY_SIZE;

        lastFPSUpdate = static_cast<float>(glfwGetTime());
        frameCount = 0;
    }
}

void DebugWindow::render(float deltaTime, float& cameraSpeed, int& renderDistance, bool& vsyncEnabled, ImVec4& clearColor, glm::vec3& cameraPos, glm::vec3& cameraFront) {
    if (!enabled) return;

    ImGui::Begin("Debug Window", nullptr, ImGuiWindowFlags_NoTitleBar);

    if (ImGui::BeginTabBar("TabBar")) {
        if (ImGui::BeginTabItem("General")) {
            ImGui::Text("Sky Color:");
            ImGui::ColorEdit3("##skyColor", (float*)&clearColor);
            ImGui::Text("Speed");
            ImGui::SliderFloat("Speed", &cameraSpeed, 0, 100);
            ImGui::Text(std::format("Camera Pos: X:{:.1f} Y:{:.1f} Z:{:.1f}", cameraPos.x, cameraPos.y, cameraPos.z).c_str());
            ImGui::Text(std::format("Camera Front: X:{:.1f} Y:{:.1f} Z:{:.1f}", cameraFront.x, cameraFront.y, cameraFront.z).c_str());
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Debugger")) {
            if (ImGui::Checkbox("VSync", &vsyncEnabled))
                glfwSwapInterval(vsyncEnabled ? 1 : 0);

            ImGui::Text(std::format("DeltaTime: {:.6f}", deltaTime).c_str());
            ImGui::Text(std::format("FPS: {:.1f}", fpsValues[fpsIndex]).c_str());

            float sum = 0.0f;
            for (int i = 0; i < FPS_HISTORY_SIZE; ++i)
                sum += fpsValues[i];

            auto [minIt, maxIt] = std::minmax_element(fpsValues, fpsValues + FPS_HISTORY_SIZE);
            char overlay[32];
            sprintf(overlay, "avg %.1f", sum / FPS_HISTORY_SIZE);

            ImGui::PlotLines("##fpsPlot", fpsValues, FPS_HISTORY_SIZE, fpsIndex, overlay, *minIt * 0.95f, *maxIt * 1.05f, ImVec2(0, 80.0f));
            ImGui::SliderInt("Render Distance", &renderDistance, 1, 100);
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

void DebugWindow::renderImGui() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
