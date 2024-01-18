#ifndef NBODY3D_RENDER_H
#define NBODY3D_RENDER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "camera.h"
#include "shader.h"
#include "sphere.h"
#include "octree.h"
#include "bhtree.h"

class Render
{
public:
    Render(int scrWidth, int scrHeight);

    void cameraSetup(Camera& worldCamera);

    void draw(Shader& sphereShader, SimulationData& data, Octree& tree);

private:
    void frameBufferSizeCallback(int width, int height);

    void scrollCallback(float xOffset, float yOffset);

    void mouseCallback(double xPosIn, double YPosIn);

    void processInput(GLFWwindow* pWindow);

    GLFWwindow * window;

    std::vector<Sphere> spheres;

    Camera camera;

    bool firstMouse = true;

    int SCR_WIDTH, SCR_HEIGHT;

    float lastX = static_cast<float>(SCR_WIDTH) / 2.0f;
    float lastY = static_cast<float>(SCR_HEIGHT) / 2.0f;

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

};

#endif //NBODY3D_RENDER_H
