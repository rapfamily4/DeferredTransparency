// Written by Luigi Rapetta, 2022.
// Thanks to learnopengl.com for the original structure.

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/transform.hpp> 
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <iostream>

#include "camera.hpp"
#include "context_manager.hpp"
#include "input/input_manager.hpp"


// --- Constructor
Camera::Camera(glm::vec3 position, unsigned int framebufferWidth, unsigned int framebufferHeight, float fov, float far, float near) :
    m_position{ position },
    m_worldUp{ CAMERA_DEFAULT_WORLDUP },
    m_front{ CAMERA_DEFAULT_FRONT },
    m_yaw{ CAMERA_DEFAULT_YAW },
    m_pitch{ CAMERA_DEFAULT_PITCH },
    m_fov{ fov },
    m_height{ (float)framebufferHeight },
    m_width{ (float)framebufferWidth },
    m_aspectRatio{ (float)framebufferWidth / (float)framebufferHeight },
    m_zFar{ far },
    m_zNear{ near },
    m_movementSpeed{ CAMERA_DEFAULT_SPEED },
    m_mouseSensitivity{ CAMERA_DEFAULT_SENSITIVITY } {
    // ---
    setResolution(m_width, m_height);
    updateCameraVectors();
}

// --- Public methods
void Camera::updateCameraVectors() {
    // Calculate the new front vector
    glm::vec3 front;
    front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    front.y = sin(glm::radians(m_pitch));
    front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    m_front = glm::normalize(front);

    // Also re-calculate the right and up vector
    m_right = glm::normalize(glm::cross(m_front, m_worldUp));
    m_up = glm::normalize(glm::cross(m_right, m_front));
    
    // Update view matrix
    updateViewMatrix();
}

void Camera::updateViewMatrix() {
    m_view = glm::lookAt(m_position, m_position + m_front, m_up);
}

void Camera::updateProjectionMatrix() {
    m_perspective = glm::perspective(glm::radians(m_fov), m_aspectRatio, m_zNear, m_zFar);
}

void Camera::updateOrthographicMatrix() {
    m_ortho = glm::ortho(0.0f, m_width, 0.0f, m_height, -m_zFar, m_zFar);
}

glm::mat4 const& Camera::getViewMatrix() const {
    return m_view;
}

glm::mat4 const& Camera::getPerspectiveMatrix() const {
    return m_perspective;
}

glm::mat4 const& Camera::getOrthoMatrix() const {
    return m_ortho;
}

glm::vec3 const& Camera::getPosition() const {
    return m_position;
}

glm::vec3 const& Camera::getFront() const {
    return m_front;
}

float Camera::getYaw() const {
    return m_yaw;
}

float Camera::getPitch() const {
    return m_pitch;
}

float Camera::getFov() const {
    return m_fov;
}

float Camera::getFarPlane() const {
    return m_zFar;
}

float Camera::getNearPlane() const {
    return m_zNear;
}

void Camera::setResolution(float width, float height) {
    m_height = height;
    m_width = width;
    m_aspectRatio = width / height;
    updateProjectionMatrix();
    updateOrthographicMatrix();
}

void Camera::setFov(float fov) {
    m_fov = fov;
    updateProjectionMatrix();
}

void Camera::setFarPlane(float far) {
    m_zFar = far;
    updateProjectionMatrix();
    updateOrthographicMatrix();
}

void Camera::setNearPlane(float near) {
    m_zNear = near;
    updateProjectionMatrix();
    updateOrthographicMatrix();
}

void Camera::keyboardHandler(int key, KeyboardType type, float deltaTime) {
    float velocity = m_movementSpeed * deltaTime;
    switch (key) {
        default:
            break;
        case GLFW_KEY_W:
            if(type == KeyboardType::hold) {
                m_position += m_front * velocity;
                updateViewMatrix();
            }
            break;
        case GLFW_KEY_S:
            if(type == KeyboardType::hold) {
                m_position -= m_front * velocity;
                updateViewMatrix();
            }
            break;
        case GLFW_KEY_A:
            if(type == KeyboardType::hold) {
                m_position -= m_right * velocity;
                updateViewMatrix();
            }
            break;
        case GLFW_KEY_D:
            if(type == KeyboardType::hold) {
                m_position += m_right * velocity;
                updateViewMatrix();
            }
            break;
        case GLFW_KEY_E:
            if(type == KeyboardType::hold) {
                m_position += m_up * velocity;
                updateViewMatrix();
            }
            break;
        case GLFW_KEY_Q:
            if(type == KeyboardType::hold) {
                m_position -= m_up * velocity;
                updateViewMatrix();
            }
            break;
    }
}

void Camera::mouseDeltaHandler(float xdelta, float ydelta, float deltaTime) {
    // Weight delta with mouse sensitivity
    float x = xdelta * m_mouseSensitivity;
    float y = ydelta * m_mouseSensitivity;

    // Update camera angles
    m_yaw += x;
    m_pitch += y;
    if (m_pitch > 89.0f)
        m_pitch = 89.0f;
    if (m_pitch < -89.0f)
        m_pitch = -89.0f;

    // Update camera vectors
    updateCameraVectors();
}

void Camera::mouseScrollHandler(float xdelta, float ydelta, float deltaTime) {
    // Update FOV
    m_fov -= (float)ydelta;

    // Clamp FOV
    if (m_fov < 1.0f)
        m_fov = 1.0f;
    if (m_fov > CAMERA_DEFAULT_FOV)
        m_fov = CAMERA_DEFAULT_FOV;

    // Update projection matrix
    updateProjectionMatrix();
}