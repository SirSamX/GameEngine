#pragma once
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

class Camera {
public:
    Camera();

    glm::vec3 pos = glm::vec3(16.5f, 130.0f, 0.5f);
    glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f,  0.0f);
    float yaw = -90.0f;
    float pitch = 0.0f;
    float fov = 45.0f;
    float speed = 15.0f;

    bool cursorLock = true;
    bool firstMouse = true;
    
    void creativeMovement(float deltaTime);
    void mouseLook(double xposIn, double yposIn);
private:
    float lastX = 800.0f / 2.0;
    float lastY = 600.0 / 2.0;
};