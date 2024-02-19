#include <iostream>
#include "octree.h"
#include "bhtree.h"

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
uint64_t mortonIndex[MAX_PARTICLES];

uint64_t nodeMortonIndex[MAX_NODES];

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
            if (depth > 4)
                return;
            printNode(nodeChildren[nodeIndex][i], depth + 1);
        }
    }
}

int main()
{
    particleX[0] = -10000.0f;
    particleY[0] = -10000.0f;
    particleZ[0] = -10000.0f;
    particleMass[0] = 1.0f;

    particleX[1] = 10000.0f;
    particleY[1] = 10000.0f;
    particleZ[1] = 10000.0f;
    particleMass[1] = 1.0f;

    particleX[4] = 5000.0f;
    particleY[4] = 5000.0f;
    particleZ[4] = 5000.0f;
    particleMass[4] = 1.0f;

    particleX[5] = 10000.0f;
    particleY[5] = 5000.0f;
    particleZ[5] = 5000.0f;
    particleMass[5] = 1.0f;
//
//    particleX[6] = -5000.0f;
//    particleY[6] = -5000.0f;
//    particleZ[6] = -5000.0f;
//    particleMass[6] = 1.0f;
//
    particleX[2] = -10000.0f;
    particleY[2] = 10000.0f;
    particleZ[2] = 10000.0f;
    particleMass[2] = 1.0f;

    particleX[3] = 10000.0f;
    particleY[3] = -10000.0f;
    particleZ[3] = -10000.0f;
    particleMass[3] = 1.0f;

//    particleX[7] = 32327.95f; particleY[7] = -9754.81f; particleZ[7] = 6090.91f; particleMass[7] = 1.0f;
//    particleX[8] = -24962.18f; particleY[8] = 32054.61f; particleZ[8] = -32190.48f; particleMass[8] = 1.0f;
//    particleX[9] = 14430.21f; particleY[9] = 22157.90f; particleZ[9] = -2960.09f; particleMass[9] = 1.0f;
//    particleX[10] = -24363.30f; particleY[10] = -6337.49f; particleZ[10] = -15083.90f; particleMass[10] = 1.0f;
//    particleX[11] = 17041.84f; particleY[11] = 15575.50f; particleZ[11] = 25853.56f; particleMass[11] = 1.0f;
//    particleX[12] = -12538.57f; particleY[12] = 23237.96f; particleZ[12] = -19492.03f; particleMass[12] = 1.0f;
//    particleX[13] = -20370.03f; particleY[13] = -7945.42f; particleZ[13] = 25188.28f; particleMass[13] = 1.0f;
//    particleX[14] = 21400.56f; particleY[14] = 17892.08f; particleZ[14] = -14832.05f; particleMass[14] = 1.0f;
//    particleX[15] = -14239.26f; particleY[15] = -29357.75f; particleZ[15] = 779.64f; particleMass[15] = 1.0f;
    //particleX[16] = -27288.70f; particleY[16] = 26340.22f; particleZ[16] = 21976.93f; particleMass[16] = 1.0f;



    SimulationData data{};

    data.particleX = particleX;
    data.nodeX = nodeX;
    data.nodeY = nodeY;
    data.nodeZ = nodeZ;
    data.nodeWidth = nodeWidth;
    data.nodeHeight = nodeHeight;
    data.nodeDepth = nodeDepth;
    //data.nodeParticleIndex = nodeParticleIndex;
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

    data.mortonIndex = mortonIndex;

    data.nodeMortonIdx = nodeMortonIndex;

    Octree tree;

    int root =  tree.buildTree(data, 6);

    //printNode(root);
    std::cout<<std::endl;

    //updateAllParticles(0.98f, 0.0f, data);
    //tree.buildTree(data);
    //printNode(0);

    return 0;
}