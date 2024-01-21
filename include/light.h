#ifndef NBODY3D_LIGHT_H
#define NBODY3D_LIGHT_H

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader.h"

class Light
{
public:
    Light(const glm::vec3 &pos, const glm::vec3 &diff,
          const glm::vec3 &spec, const glm::vec3 &ambi);

    void draw(Shader &shader);

private:
    glm::vec3 position;

    glm::vec3 diffuse;
    glm::vec3 ambient;
    glm::vec3 specular;
};

#endif //NBODY3D_LIGHT_H
