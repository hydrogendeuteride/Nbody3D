#ifndef NBODY3D_SVOTREE_H
#define NBODY3D_SVOTREE_H

#include "simulationdata.h"
#include <cstdint>

struct pair
{
    int x;
    int y;
};

struct OctreeSplit
{
    int split[8];
};

class SVOctree
{
private:
    int findSplit(ParticleData &data, int first, int last);

    void nodeChildrenInit(ParticleData &data);

public:
    int buildTree(ParticleData &data, int numParticles);

    int generateNode(ParticleData &data, NodeData &nodeData, int numParticles);

    static pair determineRange(ParticleData &data, int numParticles, int idx);
};


#endif //NBODY3D_SVOTREE_H
