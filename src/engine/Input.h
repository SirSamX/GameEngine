#pragma once
#include <unordered_map>
#include <GLFW/glfw3.h>

class Input {
public:
    Input() = delete;

    static void updateKeys(GLFWwindow* window);
    
    static bool keyJustPressed(int key);
    static bool keyJustReleased(int key);
    static bool keyPressed(int key);
private:
    struct KeyState {
        bool pressed = false;
        bool justPressed = false;
        bool justReleased = false;
    };

    static std::unordered_map<int, KeyState> keys;
};
