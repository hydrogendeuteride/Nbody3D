#ifndef NBODY3D_SIMULATIONDATA_H
#define NBODY3D_SIMULATIONDATA_H

#include <cstdint>

constexpr unsigned int OCT_CHILD = 8;

struct SimulationData
{
    float *nodeX;
    float *nodeY;
    float *nodeZ;

    float *nodeWidth;
    float *nodeHeight;
    float *nodeDepth;

    int *nodeParticleIndex;
    int (*nodeChildren)[OCT_CHILD];

    float *nodeTotalMass;
    float *nodeCOM_X;
    float *nodeCOM_Y;
    float *nodeCOM_Z;

    float *particleX;
    float *particleY;
    float *particleZ;

    float *particleVelX;
    float *particleVelY;
    float *particleVelZ;

    float *particleMass;
    float *accX;
    float *accY;
    float *accZ;

    unsigned int *idxSorted;

    uint64_t *nodeMortonCode;
};

constexpr int NULL_INDEX = -1;

constexpr int MAX_NODES = 1000;
constexpr int MAX_PARTICLES = 1000;

constexpr float THETA = 0.f;

#endif //NBODY3D_SIMULATIONDATA_H
