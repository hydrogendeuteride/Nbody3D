#ifndef NBODY3D_SIMULATIONDATA_H
#define NBODY3D_SIMULATIONDATA_H

#include <cstdint>

constexpr unsigned int OCT_CHILD = 8;

struct ParticleData
{
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

    unsigned int *idxSorted;    //store morton order sorted index of particle
    uint64_t *mortonCode;       //store sorted morton code
};

struct NodeData
{
    unsigned int (*internalNodeChildren)[OCT_CHILD];

    float *nodeTotalMass;
    float *nodeCOM_X;
    float *nodeCOM_Y;
    float *nodeCOM_Z;

    uint64_t *internalNodeMortonCode;
    uint64_t *leafNodeMortonCode;

    unsigned int *leafNodes;
    unsigned int *internalNodes;

    int *nodeDepth;
};

constexpr int NULL_INDEX = -1;

constexpr int MAX_NODES = 1000;
constexpr int MAX_PARTICLES = 1000;

constexpr float THETA = 0.5;

#endif //NBODY3D_SIMULATIONDATA_H
