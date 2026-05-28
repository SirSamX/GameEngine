#include "Input.h"
#include <algorithm>

std::unordered_map<int, Input::KeyState> Input::keys;

void Input::updateKeys(GLFWwindow* window) {
    for (auto& [key, state] : keys) {
        const bool isPressedNow = glfwGetKey(window, key) == GLFW_PRESS;
        state.justPressed = isPressedNow && !state.pressed;
        state.justReleased = !isPressedNow && state.pressed;
        state.pressed = isPressedNow;
    }
}

bool Input::keyJustPressed(const int key) {
    return keys[key].justPressed;
}

bool Input::keyJustReleased(const int key) {
    return keys[key].justReleased;
}

bool Input::keyPressed(const int key) {
    return keys[key].pressed;
}