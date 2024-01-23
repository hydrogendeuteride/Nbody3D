#include "sphere.h"

Sphere::Sphere(int subdivision, float size)
{
    buildSphere(subdivision, size);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), static_cast<void *>(vertices.data()),
                 GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof (int), indices.data(), GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) nullptr);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float )));
    glEnableVertexAttribArray(1);
}

void Sphere::buildSphere(int subdivision, float size)
{
    const float DEG2RAD = std::acos(-1.0f) / 180.0f;

    float n1[3];
    float n2[3];
    float v[3];
    float a1;
    float a2;

    int pointsPerRow = (int) pow(2, subdivision) + 1;

    for (int i = 0; i < pointsPerRow; ++i)
    {
        a2 = DEG2RAD * (90.0f - 180.0f * (float) i / (float) (pointsPerRow - 1));
        n2[0] = -std::sin(a2);
        n2[1] = std::cos(a2);
        n2[2] = 0;

        for (int j = 0; j < pointsPerRow; ++j)
        {
            a1 = DEG2RAD * (360.0f * (float) j / (float) (pointsPerRow - 1));
            n1[0] = -std::sin(a1);
            n1[1] = 0;
            n1[2] = -std::cos(a1);

            v[0] = n1[1] * n2[2] - n1[2] * n2[1];
            v[1] = n1[2] * n2[0] - n1[0] * n2[2];
            v[2] = n1[0] * n2[1] - n1[1] * n2[0];

            float scale = 1 / std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
            v[0] *= scale * size;
            v[1] *= scale * size;
            v[2] *= scale * size;

            vertices.push_back(v[0]);
            vertices.push_back(v[1]);
            vertices.push_back(v[2]);
            vertices.push_back(v[0]);//normal
            vertices.push_back(v[1]);
            vertices.push_back(v[2]);
        }
    }

    for (int i = 0; i < pointsPerRow - 1; ++i)
    {
        for (int j = 0; j < pointsPerRow - 1; ++j)
        {
            int first = i * pointsPerRow + j;
            int second = first + pointsPerRow;

            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);

            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }
}

void Sphere::draw(const Shader &shader, const glm::vec3& diffuse, const glm::vec3& specular,
                  const glm::vec3& emit, const glm::vec3& ambient, const Sphere::DrawType drawType)
{
    shader.setVec3("objDiff", diffuse);
    shader.setVec3("objSpec", specular);
    shader.setVec3("objAmbi", ambient);
    shader.setVec3("objEmit", emit);

    shader.setMat4("model", worldMatrix);

    shader.use();

    glBindVertexArray(VAO);

    switch (drawType)
    {
        case DrawType::POINTS:
            glDrawElements(GL_POINTS, indices.size(), GL_UNSIGNED_INT, nullptr);
            break;
        case DrawType::LINES:
            glDrawElements(GL_LINES, indices.size(), GL_UNSIGNED_INT, nullptr);
            break;
        case DrawType::TRIANGLES:
            glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);
            break;
        case DrawType::PATCHES:
            glDrawElements(GL_PATCHES, indices.size(), GL_UNSIGNED_INT, nullptr);
            break;
    }
}