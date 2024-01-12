#ifndef NBODY3D_CAMERA_H
#define NBODY3D_CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <vector>

enum CameraMovement
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

constexpr float YAW = -90.0f;
constexpr float PITCH = 0.0f;
constexpr float ROLL = 0.0f;
constexpr float SPEED = 2.5f;
constexpr float SENSITIVITY = 0.1f;
constexpr float ZOOM = 45.0f;

class Camera
{
public:
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;

    float yaw, pitch, roll;
    float movementSpeed;

    float mouseSensitivity;
    float zoom;

    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH, float roll = ROLL)
            : front(glm::vec3(0.0f, 1.0f, 0.0f))
    {
        this->position = position;
        this->worldUp = up;
        this->yaw = yaw;
        this->pitch = pitch;
        updateCameraVectors();
    }

    glm::mat4 getViewMatrix()
    {
        return glm::lookAt(position, position + front, up);
    }

    void processKeyboard(CameraMovement direction, float deltaTime)
    {
        float velocity = movementSpeed * deltaTime;
        if(direction==FORWARD)
            position += front * velocity;
        if(direction==BACKWARD)
            position -= front * velocity;
        if(direction==LEFT)
            position -= right * velocity;
        if(direction==RIGHT)
            position += right * velocity;
        if(direction==UP)
            position += up * velocity;
        if(direction==FORWARD)
            position -= up * velocity;
    }

    void processMouseMovement(float xOffset, float yOffset)
    {
        xOffset *= mouseSensitivity;
        yOffset *= mouseSensitivity;

        yaw += xOffset;
        pitch += yOffset;

        updateCameraVectors();
    }

    void processMouseScroll(float yOffset)
    {
        zoom-= yOffset;
        if (zoom < 1.0f)
            zoom =1.0f;
        if (zoom > 45.0f)
            zoom = 45.0f;
    }

private:
    void updateCameraVectors()
    {
        glm::quat qYaw = glm::angleAxis(glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::quat qPitch = glm::angleAxis(glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::quat qRoll = glm::angleAxis(glm::radians(roll), glm::vec3(0.0f, 0.0f, 1.0f));

        glm::quat orientation = qYaw * qPitch * qRoll;
        orientation = glm::normalize(orientation);

        front = glm::rotate(orientation, glm::vec3(0.0f, 0.0f, -1.0f));

        right = glm::normalize(glm::cross(front, worldUp));
        up = glm::normalize(glm::cross(right, front));
    }
};

#endif //NBODY3D_CAMERA_H
