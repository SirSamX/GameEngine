#include "DebugWindow.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <algorithm>
#include <format>
#include <GLFW/glfw3.h>

#include "Time.h"

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

void DebugWindow::updateFps() {
    frameCount++;
    if (gameTime.getTotalTime() - lastFPSUpdate >= 0.5f) {
        const float fps = static_cast<float>(frameCount) / (gameTime.getTotalTime() - lastFPSUpdate);
        fpsValues[fpsIndex] = fps;
        fpsIndex = (fpsIndex + 1) % FPS_HISTORY_SIZE;

        lastFPSUpdate = gameTime.getTotalTime();
        frameCount = 0;
    }
}

void DebugWindow::render(float& cameraSpeed, int& renderDistance, bool& vsyncEnabled, glm::vec3& clearColor, const glm::vec3& cameraPos, const glm::vec3& cameraFront, glm::vec3& lightColor) {
    if (!enabled) return;

    ImGui::Begin("Debug Window", nullptr, ImGuiWindowFlags_NoTitleBar);

    if (ImGui::BeginTabBar("TabBar")) {
        if (ImGui::BeginTabItem("General")) {
            ImGui::Text("Sky Color:");
            ImGui::ColorEdit3("##skyColor", &clearColor[0]);
            ImGui::Text("Light Color:");
            ImGui::ColorEdit3("##lightColor", &lightColor[0]);
            ImGui::Text("Speed");
            ImGui::SliderFloat("Speed", &cameraSpeed, 0, 100);
            ImGui::Text("Camera Pos: X:%.1f Y:%.1f Z:%.1f", cameraPos.x, cameraPos.y, cameraPos.z);
            ImGui::Text("Camera Front: X:%.1f Y:%.1f Z:%.1f", cameraFront.x, cameraFront.y, cameraFront.z);
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Debugger")) {
            if (ImGui::Checkbox("VSync", &vsyncEnabled))
                glfwSwapInterval(vsyncEnabled ? 1 : 0);

            ImGui::Text("DeltaTime: %.6f", gameTime.getDeltaTime());
            ImGui::Text("FPS: %.1f", fpsValues[fpsIndex]);

            float sum = 0.0f;
            for (const float fpsValue : fpsValues)
                sum += fpsValue;

            auto [minIt, maxIt] = std::minmax_element(fpsValues, fpsValues + FPS_HISTORY_SIZE);
            char overlay[32];
            sprintf(overlay, "avg %.1f", sum / FPS_HISTORY_SIZE);

            ImGui::PlotLines("##fpsPlot", fpsValues, FPS_HISTORY_SIZE, fpsIndex, overlay, *minIt * 0.95f, *maxIt * 1.05f, ImVec2(0, 80.0f));
            ImGui::SliderInt("Render Distance", &renderDistance, 1, 100);
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Models")) {
            if (ImGui::Button("Backpack")) {
            }
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
