#ifndef NBODY3D_SIMULATIONDATA_H
#define NBODY3D_SIMULATIONDATA_H

struct SimulationData
{
    float *nodeX;
    float *nodeY;
    float *nodeWidth;
    float *nodeHeight;
    int *nodeParticleIndex;
    int (*nodeChildren)[4];

    float *nodeTotalMass;
    float *nodeCOM_X;
    float *nodeCOM_Y;

    float *particleX;
    float *particleY;
    float *particleVelX;
    float *particleVelY;
    float *particleMass;
    float *accX;
    float *accY;

    unsigned int *idxSorted;
};

constexpr int NULL_INDEX = -1;

constexpr int MAX_NODES = 1000;
constexpr int MAX_PARTICLES = 1000;

constexpr float THETA = 0.5;

#endif //NBODY3D_SIMULATIONDATA_H
