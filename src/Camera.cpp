#include "Camera.h"
#include "Input.h"

Camera::Camera() {

}

void Camera::creativeMovement(float deltaTime) {
    float camSpeed = speed * deltaTime;
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

void Camera::mouseLook(double xposIn, double yposIn) {
    if (!cursorLock) return;

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(direction);
}