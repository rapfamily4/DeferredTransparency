// Written by Luigi Rapetta, 2022.
// Thanks to learnopengl.com for the original structure.

#ifndef CAMERA_H
#define CAMERA_H

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

#include "consts.hpp"
#include "input/input_manager.hpp"


// --- Camera class
class Camera {
public:
    // --- Constructor
    Camera(glm::vec3 position = CAMERA_DEFAULT_POSITION, unsigned int framebufferWidth = SCREEN_DEFAULT_WIDTH, unsigned int framebufferHeight = SCREEN_DEFAULT_HEIGHT, float fov = CAMERA_DEFAULT_FOV, float far = CAMERA_DEFAULT_ZFAR, float near = CAMERA_DEFAULT_ZNEAR);

    // --- Public methods
    void updateCameraVectors();
    void updateViewMatrix();
    void updateProjectionMatrix();
    void updateOrthographicMatrix();
    glm::mat4 const& getViewMatrix() const;
    glm::mat4 const& getPerspectiveMatrix() const;
    glm::mat4 const& getOrthoMatrix() const;
    glm::vec3 const& getPosition() const;
    glm::vec3 const& getFront() const;
    float getYaw() const;
    float getPitch() const;
    float getFov() const;
    float getFarPlane() const;
    float getNearPlane() const;
    void setResolution(float width, float height);
    void setFov(float fov);
    void setFarPlane(float far);
    void setNearPlane(float near);
    void keyboardHandler(int key, KeyboardType type, float deltaTime);
    void mouseDeltaHandler(float xdelta, float ydelta, float deltaTime);
    void mouseScrollHandler(float xdelta, float ydelta, float deltaTime);

private:
    // --- Private members
    glm::vec3 m_position;
    glm::vec3 m_worldUp;
    glm::vec3 m_up;
    glm::vec3 m_right;
    glm::vec3 m_front;
    float m_yaw;
    float m_pitch;
    float m_fov;
    float m_height;
    float m_width;
    float m_aspectRatio;
    float m_zFar;
    float m_zNear;
    glm::mat4 m_view;
    glm::mat4 m_perspective;
    glm::mat4 m_ortho;
    float m_movementSpeed;
    float m_mouseSensitivity;
};


#endif // CAMERA_H
