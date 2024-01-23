#include <iostream>
#include "octree.h"
#include "bhtree.h"
#include "render.h"

float nodeX[MAX_NODES];
float nodeY[MAX_NODES];
float nodeZ[MAX_NODES];

float nodeWidth[MAX_NODES];
float nodeHeight[MAX_NODES];
float nodeDepth[MAX_NODES];

int nodeParticleIndex[MAX_NODES];
int nodeChildren[MAX_NODES][OCT_CHILD];

float nodeTotalMass[MAX_NODES];
float nodeCOM_X[MAX_NODES];
float nodeCOM_Y[MAX_NODES];
float nodeCOM_Z[MAX_NODES];

float particleX[MAX_PARTICLES];
float particleY[MAX_PARTICLES];
float particleZ[MAX_PARTICLES];

float particleVelX[MAX_PARTICLES];
float particleVelY[MAX_PARTICLES];
float particleVelZ[MAX_PARTICLES];
float particleMass[MAX_PARTICLES];
float accX[MAX_PARTICLES];
float accY[MAX_PARTICLES];
float accZ[MAX_PARTICLES];

unsigned int idxSorted[MAX_PARTICLES];

void printNode(int nodeIndex, int depth = 0)
{
    for (int i = 0; i < depth; i++)
    {
        std::cout << "  ";
    }

    std::cout << "Node [" << nodeX[nodeIndex] << ", " << nodeY[nodeIndex] << ", " << nodeZ[nodeIndex] << ", "
              << nodeWidth[nodeIndex] << ", " << nodeHeight[nodeIndex] << ", " << nodeDepth[nodeIndex] << ", "
              << nodeTotalMass[nodeIndex] << "]";

    if (nodeParticleIndex[nodeIndex] != NULL_INDEX)
    {
        std::cout << " Particle: (" << particleX[nodeParticleIndex[nodeIndex]] << ", "
                  << particleY[nodeParticleIndex[nodeIndex]] << ", "
                  << particleZ[nodeParticleIndex[nodeIndex]] << ")";
    }

    std::cout << std::endl;

    for (int i = 0; i < 8; i++)
    {
        if (nodeChildren[nodeIndex][i] != NULL_INDEX)
        {
            printNode(nodeChildren[nodeIndex][i], depth + 1);
        }
    }
}

int main()
{
    particleX[0] = 0.0f;
    particleY[0] = 0.0f;
    particleZ[0] = 0.0f;
    particleMass[0] = 1.0f;

    SimulationData data{};

    data.particleX = particleX;
    data.nodeX = nodeX;
    data.nodeY = nodeY;
    data.nodeZ = nodeZ;
    data.nodeWidth = nodeWidth;
    data.nodeHeight = nodeHeight;
    data.nodeDepth = nodeDepth;
    data.nodeParticleIndex = nodeParticleIndex;
    data.nodeChildren = nodeChildren;

    data.nodeTotalMass = nodeTotalMass;
    data.nodeCOM_X = nodeCOM_X;
    data.nodeCOM_Y = nodeCOM_Y;
    data.nodeCOM_Z = nodeCOM_Z;

    data.particleX = particleX;
    data.particleY = particleY;
    data.particleZ = particleZ;
    data.particleVelX = particleVelX;
    data.particleVelY = particleVelY;
    data.particleVelZ = particleVelZ;
    data.particleMass = particleMass;
    data.accX = accX;
    data.accY = accY;
    data.accZ = accZ;
    data.idxSorted = idxSorted;

    Octree tree;

    Render render(1920, 1080);

    Shader shader("../shader/shader.vert", "../shader/shader.frag");

    Camera camera(glm::vec3(0.0f, 0.0f, -3.0f));

    Light light(glm::vec3 (1.0f, 1.0f, 1.0f), glm::vec3 (1.0f, 1.0f, 1.0f),
                glm::vec3 (0.5f, 0.5f, 0.5f), glm::vec3 (0.2f, 0.2f, 0.2f));

    render.lightSetup(light);

    render.sphereSetup(6, 0.1f, 1);

    render.cameraSetup(camera);

    render.draw(shader, data, tree);

    return 0;
}