#include "light.h"

Light::Light(const glm::vec3& pos, const glm::vec3& diff,
             const glm::vec3& spec, const glm::vec3& ambi)
             :position(pos), diffuse(diff), ambient(ambi), specular(spec)
{

}

void Light::draw(Shader &shader)
{
    shader.setVec3("lightPos", position);

    shader.setVec3("lightDiff", diffuse);
    shader.setVec3("lightAmbi", ambient);
    shader.setVec3("lightSpec", specular);
}