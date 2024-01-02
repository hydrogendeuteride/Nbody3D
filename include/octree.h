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

    int nodeCount = 0;

    int generateNode(SimulationData &data);

    void nodeCOMInit(SimulationData &data);

public:
    int buildTree(SimulationData &data);
};

#endif //NBODY3D_OCTREE_H
