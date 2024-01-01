#ifndef NBODY3D_OCTREE_H
#define NBODY3D_OCTREE_H

#include "simulationdata.h"

class Octree
{
private:
    int createNode(float x, float y, float z, float width, float height, float depth,
                   const SimulationData &data);

    static bool noChildren(const SimulationData &data, int nodeIndex);

    struct pair
    {
        int node;
        int particle;
    };

    void insertParticleToNode(int nodeIndex, int particleIndex, const SimulationData &data);

    int nodeCount = 0;

    void generateNode(SimulationData &data, int first, int last);

public:
    void buildTree(const SimulationData &data);
};

#endif //NBODY3D_OCTREE_H
