#ifndef NBODY3D_OCTREE_H
#define NBODY3D_OCTREE_H

#include "simulationdata.h"

class Octree
{
private:
    static unsigned int expandBits(unsigned int v);

    static unsigned int morton2D(float x, float y);

    int createNode(float x, float y, float width, float height, const SimulationData &data);

    struct pair
    {
        int node;
        int particle;
    };

    void insertParticleToNode(int nodeIndex, int particleIndex, const SimulationData &data);

    int nodeCount = 0;

public:
    void buildTree(const SimulationData &data);
};

#endif //NBODY3D_OCTREE_H
