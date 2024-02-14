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
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "NBodySim", nullptr, nullptr);
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
        win ->mouseMovementCallback(xOffset, yOffset);
    });

    glfwSetMouseButtonCallback(window, [](GLFWwindow *w, int button, int action, int mods) {
        auto *win = static_cast<Render*>(glfwGetWindowUserPointer(w));
        win->mouseButtonCallback(button, action, mods);
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

void Render::sphereSetup(int subdivision, float size, int number)
{
    for (int i = 0; i < number; ++i)
        spheres.emplace_back(subdivision, size);
}

void Render::lightSetup(Light &light)
{
    this->lightSource = &light;
}

void Render::frameBufferSizeCallback(int width, int height)
{
    glViewport(0, 0, width, height);
}

void Render::scrollCallback(float xOffset, float yOffset)
{
    camera.processMouseScroll(static_cast<float>(yOffset));
}

void Render::mouseButtonCallback(int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        if (action == GLFW_PRESS)
        {
            isDragging = true;
            double mouseX, mouseY;
            glfwGetCursorPos(window, &mouseX, &mouseY);
            lastMousePosition = glm::vec2(mouseX, mouseY);
        }
        else if (action == GLFW_RELEASE)
        {
            isDragging = false;
        }
    }
}

void Render::mouseMovementCallback(double xPosIn, double yPosIn)
{
    float xpos = static_cast<float>(xPosIn);
    float ypos = static_cast<float>(yPosIn);

    if (isDragging)
    {
        glm::vec2 currentMousePosition = glm::vec2(xpos, ypos);
        glm::vec2 mouseDelta = currentMousePosition - lastMousePosition;

        camera.processMouseMovement(mouseDelta.x, mouseDelta.y);

        lastMousePosition = currentMousePosition;
    }
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
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.processKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        camera.processKeyboard(DOWN, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.processKeyboard(ROLL_LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.processKeyboard(ROLL_RIGHT, deltaTime);
}

void Render::draw(Shader &sphereShader, SimulationData& data, Octree& tree)
{
    glm::vec3 ambient(0.1f, 0.1f, 0.1f);
    glm::vec3 diffuse(0.3f, 0.4f, 0.8f);
    glm::vec3 specular(1.0f, 1.0f, 1.0f);

    float shininess = 32.0f;

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(glm::radians(camera.zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT,
                                                0.1f, 60000.0f);
        glm::mat4 view = camera.getViewMatrix();

        sphereShader.setMat4("projection", projection);
        sphereShader.setMat4("view", view);

        sphereShader.setVec3("viewPos", camera.position);

        lightSource->draw(sphereShader);

        processInput(window);

        tree.buildTree(data);
        updateAllParticles(0.99f, deltaTime * 0.01f, data);

        for (size_t i = 0; i < spheres.size(); ++i)
        {
            spheres[i].worldMatrix = glm::translate(glm::mat4(1.0f),
                                                    glm::vec3(data.particleX[i],
                                                              data.particleY[i],
                                                              data.particleZ[i]));
            std::cout << data.accX[i] <<" "<< data.accY[i] <<" "<< data.accZ[i] << "\n";
            spheres[i].draw(sphereShader, diffuse, specular, ambient,
                            glm::vec3 (0.0f, 0.0f, 0.0f));

            std::cout << data.accX[i] <<" "<< data.accY[i] <<" "<< data.accZ[i] << "\n";
        }
        std::cout << "\n";

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}