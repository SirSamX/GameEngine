#include "Camera.h"
#include "Input.h"
#include "Time.h"

Camera::Camera() = default;

void Camera::creativeMovement() {
    float camSpeed = speed * gameTime.getDeltaTime();
    if (Input::keyPressed(GLFW_KEY_W))
        pos += camSpeed * front;
    if (Input::keyPressed(GLFW_KEY_S))
        pos -= camSpeed * front;
    if (Input::keyPressed(GLFW_KEY_A))
        pos -= glm::normalize(glm::cross(front, up)) * camSpeed;
    if (Input::keyPressed(GLFW_KEY_D))
        pos += glm::normalize(glm::cross(front, up)) * camSpeed;
    if (Input::keyPressed(GLFW_KEY_SPACE))
        pos += camSpeed * up;
    if (Input::keyPressed(GLFW_KEY_LEFT_SHIFT))
        pos -= camSpeed * up;
}

void Camera::mouseLook(const double xPosIn, const double yPosIn) {
    if (!cursorLock) return;

    const auto xPos = static_cast<float>(xPosIn);
    const auto yPos = static_cast<float>(yPosIn);

    if (firstMouse) {
        lastX = xPos;
        lastY = yPos;
        firstMouse = false;
    }

    float xOffset = xPos - lastX;
    float yOffset = lastY - yPos;
    lastX = xPos;
    lastY = yPos;

    float sensitivity = 0.1f;
    xOffset *= sensitivity;
    yOffset *= sensitivity;

    yaw += xOffset;
    pitch += yOffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 direction;
    direction.x = glm::cos(glm::radians(yaw)) * glm::cos(glm::radians(pitch));
    direction.y = glm::sin(glm::radians(pitch));
    direction.z = glm::sin(glm::radians(yaw)) * glm::cos(glm::radians(pitch));
    front = glm::normalize(direction);
}