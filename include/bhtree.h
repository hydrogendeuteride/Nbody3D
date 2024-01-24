#ifndef NBODY3D_BHTREE_H
#define NBODY3D_BHTREE_H

#include <cmath>
#include "simulationdata.h"
#include "integrator.h"

bool isParticleInNode(float pX, float pY, float pZ, float nX, float nY, float nZ, float nW, float nH, float nD);

struct vec
{
    float x;
    float y;
    float z;
};

class Gravitational
{
public:
    vec operator()(const float rootMass, const float distX, const float distY, const float distZ)
    {
        float dist = std::sqrt(distX * distX + distY * distY + distZ * distZ);

        float tmp = -((1.0f * rootMass) / std::pow((dist * dist) + (0.1f * 0.1f), 1.5f));
        return {tmp * distX, tmp * distY, tmp * distZ};
    }
};

void netAcceleration(int particleIdx, const SimulationData &data);

void boundaryDetection(int particleIdx, float offset, const SimulationData &data);

void updateAllParticles(float damping, float dt, const SimulationData &data);

#endif //NBODY3D_BHTREE_H
