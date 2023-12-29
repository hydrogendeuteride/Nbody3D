#ifndef NBODY3D_BHTREE_H
#define NBODY3D_BHTREE_H

#include <cmath>
#include "simulationdata.h"
#include "integrator.h"


bool isParticleInNode(float pX, float pY, float nX, float nY, float nW, float nH);

struct vec
{
    float x;
    float y;
};

class Gravitational
{
public:
    vec operator()(const float rootMass, const float distX, const float distY)
    {
        float dist = std::sqrt(distX * distX + distY * distY);

        float tmp = -((1.0f * rootMass) / std::pow((dist * dist) + (0.1f * 0.1f), 1.5f));
        return {tmp * distX, tmp * distY};
    }
};

void netAcceleration(int particleIdx, const SimulationData &data);

void boundaryDetection(int particleIdx, float offset, const SimulationData &data);

void updateAllParticles(float damping, float dt, const SimulationData &data);

#endif //NBODY3D_BHTREE_H
