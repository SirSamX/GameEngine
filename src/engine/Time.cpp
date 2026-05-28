#include "Time.h"
#include <GLFW/glfw3.h>

void Time::update() {
    const double currentFrameTime = glfwGetTime();

    deltaTime = static_cast<float>(currentFrameTime - lastFrameTime);
    totalTime = static_cast<float>(currentFrameTime);

    lastFrameTime = currentFrameTime;
}

