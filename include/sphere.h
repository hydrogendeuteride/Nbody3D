#ifndef NBODY3D_SPHERE_H
#define NBODY3D_SPHERE_H

#include <vector>
#include "glm/glm.hpp"
#include "shader.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Sphere
{
public:
    Sphere(int subdivision, float size);

    enum class DrawType { POINTS, LINES, TRIANGLES, PATCHES };
    void draw(const Shader& shader, const glm::vec3& diffuse, const glm::vec3& specular, const glm::vec3& ambient,
              const DrawType drawType = DrawType::TRIANGLES);

    glm::mat4 worldMatrix;
private:
    void buildSphere(int subdivision, float size);

    std::vector<float> vertices;
    std::vector<unsigned short> indices;

    unsigned int VAO, VBO, EBO;
};

#endif //NBODY3D_SPHERE_H
