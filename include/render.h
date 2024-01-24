#ifndef NBODY3D_RENDER_H
#define NBODY3D_RENDER_H

#include "shader.h"
#include "camera.h"
#include "sphere.h"
#include "octree.h"
#include "bhtree.h"
#include "light.h"

class Render
{
public:
    Render(int scrWidth, int scrHeight);

    void cameraSetup(Camera &worldCamera);

    void sphereSetup(int subdivision, float size, int number);
    void lightSetup(Light& light);

    void draw(Shader &sphereShader, SimulationData &data, Octree &tree);

private:
    void frameBufferSizeCallback(int width, int height);

    void scrollCallback(float xOffset, float yOffset);

    void mouseMovementCallback(double xPosIn, double YPosIn);

    void mouseButtonCallback(int button, int action, int mods);

    void processInput(GLFWwindow *pWindow);

    GLFWwindow *window;

    std::vector<Sphere> spheres;
    Light *lightSource;

    Camera camera;

    bool firstMouse = true;

    int SCR_WIDTH, SCR_HEIGHT;

    float lastX = static_cast<float>(SCR_WIDTH) / 2.0f;
    float lastY = static_cast<float>(SCR_HEIGHT) / 2.0f;

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    glm::vec2 lastMousePosition;
    bool isDragging = false;
};

#endif //NBODY3D_RENDER_H
