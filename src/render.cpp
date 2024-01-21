#include "render.h"

Render::Render(int scrWidth, int scrHeight)
{
    SCR_WIDTH = scrWidth;
    SCR_HEIGHT = scrHeight;

    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        std::exit(EXIT_FAILURE);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(scrWidth, scrHeight, "NBodySim", nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);

    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        std::exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow *w, int width, int height) {
        auto *win = static_cast<Render *>(glfwGetWindowUserPointer(w));
        win->frameBufferSizeCallback(width, height);
    });

    glfwSetScrollCallback(window, [](GLFWwindow *w, double xoffset, double yoffset) {
        auto *win = static_cast<Render *>(glfwGetWindowUserPointer(w));
        win->scrollCallback(xoffset, yoffset);
    });

    glfwSetCursorPosCallback(window, [](GLFWwindow *w, double xOffset, double yOffset){
        auto *win = static_cast<Render *>(glfwGetWindowUserPointer(w));
        win ->mouseCallback(xOffset, yOffset);
    });

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    glEnable(GL_DEPTH_TEST);
}

void Render::cameraSetup(Camera &worldCamera)
{
    this->camera = worldCamera;
}

void Render::sphereSetup(int subdivision, float size)
{

}

void Render::frameBufferSizeCallback(int width, int height)
{
    glViewport(0, 0, width, height);
}

void Render::scrollCallback(float xOffset, float yOffset)
{
    camera.processMouseScroll(static_cast<float>(yOffset));
}

void Render::mouseCallback(double xPosIn, double yPosIn)
{
    float xpos = static_cast<float>(xPosIn);
    float ypos = static_cast<float>(yPosIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.processMouseMovement(xoffset, yoffset);
}

void Render::processInput(GLFWwindow *pWindow)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.processKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.processKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.processKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.processKeyboard(RIGHT, deltaTime);
}

void Render::draw(Shader &sphereShader, SimulationData& data, Octree& tree)
{
    glm::vec3 ambient(0.1f, 0.1f, 0.1f);
    glm::vec3 diffuse(0.8f, 0.7f, 0.6f);
    glm::vec3 specular(1.0f, 1.0f, 1.0f);

    float shininess = 32.0f;

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(glm::radians(camera.zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT,
                                                0.1f, 100.0f);
        glm::mat4 view = camera.getViewMatrix();

        sphereShader.setMat4("projection", projection);
        sphereShader.setMat4("view", view);

        sphereShader.setVec3("viewPos", camera.position);

        processInput(window);

        tree.buildTree(data);
        updateAllParticles(0.98f, deltaTime, data);

        for (size_t i = 0; i < spheres.size(); ++i)
        {
            spheres[i].worldMatrix = glm::translate(glm::mat4(1.0f),
                                                    glm::vec3(data.particleX[i],
                                                              data.particleY[i],
                                                              data.particleZ[i]));

            spheres[i].draw(sphereShader, diffuse, specular, ambient,
                            glm::vec3 (0.0f, 0.0f, 0.0f));
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}