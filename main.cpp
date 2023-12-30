#include <iostream>
#include "octree.h"
#include "bhtree.h"

float nodeX[MAX_NODES];
float nodeY[MAX_NODES];
float nodeWidth[MAX_NODES];
float nodeHeight[MAX_NODES];
int nodeParticleIndex[MAX_NODES];
int nodeChildren[MAX_NODES][OCT_CHILD];

float nodeTotalMass[MAX_NODES];
float nodeCOM_X[MAX_NODES];
float nodeCOM_Y[MAX_NODES];

float particleX[MAX_PARTICLES];
float particleY[MAX_PARTICLES];
float particleVelX[MAX_PARTICLES];
float particleVelY[MAX_PARTICLES];
float particleMass[MAX_PARTICLES];
float accX[MAX_PARTICLES];
float accY[MAX_PARTICLES];

unsigned int idxSorted[MAX_PARTICLES];

void printNode(int nodeIndex, int depth = 0)
{
    for (int i = 0; i < depth; i++)
    {
        std::cout << "  ";
    }

    std::cout << "Node [" << nodeX[nodeIndex] << ", " << nodeY[nodeIndex] << ", "
              << nodeWidth[nodeIndex] << ", " << nodeHeight[nodeIndex] << ", "
              << nodeTotalMass[nodeIndex] << "]";

    if (nodeParticleIndex[nodeIndex] != NULL_INDEX)
    {
        std::cout << " Particle: (" << particleX[nodeParticleIndex[nodeIndex]] << ", "
                  << particleY[nodeParticleIndex[nodeIndex]] << ")";
    }

    std::cout << std::endl;

    for (int i = 0; i < 4; i++)
    {
        if (nodeChildren[nodeIndex][i] != NULL_INDEX)
        {
            printNode(nodeChildren[nodeIndex][i], depth + 1);
        }
    }
}

int main()
{
    particleX[0] = -10000.0f;
    particleY[0] = -10000.0f;
    particleMass[0] = 1.0f;

    particleX[1] = 10000.0f;
    particleY[1] = 10000.0f;
    particleMass[1] = 1.0f;

    particleX[4] = 5000.0f;
    particleY[4] = 5000.0f;
    particleMass[4] = 1.0f;

    particleX[5] = 10000.0f;
    particleY[5] = 5000.0f;
    particleMass[5] = 1.0f;

    particleX[6] = -5000.0f;
    particleY[6] = -5000.0f;
    particleMass[6] = 1.0f;

    particleX[2] = -10000.0f;
    particleY[2] = 10000.0f;
    particleMass[2] = 1.0f;

    particleX[3] = 10000.0f;
    particleY[3] = -10000.0f;
    particleMass[3] = 1.0f;

    SimulationData data{};

    data.particleX = particleX;
    data.nodeX = nodeX;
    data.nodeY = nodeY;
    data.nodeWidth = nodeWidth;
    data.nodeHeight = nodeHeight;
    data.nodeParticleIndex = nodeParticleIndex;
    data.nodeChildren = nodeChildren;

    data.nodeTotalMass = nodeTotalMass;
    data.nodeCOM_X = nodeCOM_X;
    data.nodeCOM_Y = nodeCOM_Y;

    data.particleX = particleX;
    data.particleY = particleY;
    data.particleVelX = particleVelX;
    data.particleVelY = particleVelY;
    data.particleMass = particleMass;
    data.accX = accX;
    data.accY = accY;
    data.idxSorted = idxSorted;

    Octree tree;

    tree.buildTree(data);

    printNode(0);

    updateAllParticles(0.98f, 0.0f, data);
    tree.buildTree(data);
    printNode(0);

    return 0;
}