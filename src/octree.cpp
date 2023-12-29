#include <algorithm>
#include <stack>
#include <numeric>
#include "octree.h"
#include "omp.h"

unsigned int Octree::expandBits(unsigned int v)
{
    v = (v | (v << 8)) & 0x00FF00FF;
    v = (v | (v << 4)) & 0x0F0F0F0F;
    v = (v | (v << 2)) & 0x33333333;
    v = (v | (v << 1)) & 0x55555555;

    return v;
}

unsigned int Octree::morton2D(float x, float y)
{
    x += 32768.0f;
    y += 32768.0f;

    auto scaledX = (unsigned int) x;
    auto scaledY = (unsigned int) y;

    scaledX = std::max(0U, std::min(scaledX, 65536U));
    scaledY = std::max(0U, std::min(scaledY, 65536U));

    unsigned int xx = expandBits(scaledX);
    unsigned int yy = expandBits(scaledY);

    return xx | (yy << 1);
}

int Octree::createNode(float x, float y, float width, float height, const SimulationData &data)
{
    int index;
#pragma omp atomic capture
    index = nodeCount++;

    data.nodeX[index] = x;
    data.nodeY[index] = y;
    data.nodeWidth[index] = width;
    data.nodeHeight[index] = height;
    data.nodeParticleIndex[index] = NULL_INDEX;
    for (int i = 0; i < 4; i++)
    {
        data.nodeChildren[index][i] = NULL_INDEX;
    }
    return index;
}

void Octree::insertParticleToNode(int nodeIndex, int particleIndex, const SimulationData &data)
{
    std::stack<pair> stack;
    stack.push({nodeIndex, particleIndex});

    while (!stack.empty())
    {
        auto top = stack.top();
        stack.pop();
        nodeIndex = top.node;
        particleIndex = top.particle;

        // If the node is a leaf node with no children, simply insert the particle
        if (data.nodeParticleIndex[nodeIndex] == NULL_INDEX && data.nodeChildren[nodeIndex][0] == NULL_INDEX)
        {
            data.nodeParticleIndex[nodeIndex] = particleIndex;
            data.nodeTotalMass[nodeIndex] = data.particleMass[particleIndex];
            data.nodeCOM_X[nodeIndex] = data.particleX[particleIndex];
            data.nodeCOM_Y[nodeIndex] = data.particleY[particleIndex];
        }
        else
        {
            float halfWidth = data.nodeWidth[nodeIndex] / 2.0f;
            float halfHeight = data.nodeHeight[nodeIndex] / 2.0f;
            // If the node already contains a particle or has children, we need to update the COM and mass
            // Generate new node for existing particle and push them to stack
            if (data.nodeParticleIndex[nodeIndex] != NULL_INDEX || data.nodeChildren[nodeIndex][0] != NULL_INDEX)
            {
                // If the node was previously a leaf node with a particle, we need to create children and redistribute
                if (data.nodeParticleIndex[nodeIndex] != NULL_INDEX)
                {
                    int existingParticleIndex = data.nodeParticleIndex[nodeIndex];
                    data.nodeParticleIndex[nodeIndex] = NULL_INDEX;

                    int childIndex = 0;
                    if (data.particleX[existingParticleIndex] > data.nodeX[nodeIndex] + halfWidth)
                    {
                        childIndex |= 1;
                    }
                    if (data.particleY[existingParticleIndex] > data.nodeY[nodeIndex] + halfHeight)
                    {
                        childIndex |= 2;
                    }

                    if (data.nodeChildren[nodeIndex][childIndex] == NULL_INDEX)
                    {
                        float childX = data.nodeX[nodeIndex] + (childIndex & 1 ? halfWidth : 0);
                        float childY = data.nodeY[nodeIndex] + (childIndex & 2 ? halfHeight : 0);
                        data.nodeChildren[nodeIndex][childIndex] = createNode(childX, childY, halfWidth, halfHeight,
                                                                              data);
                    }
                    stack.push({data.nodeChildren[nodeIndex][childIndex], existingParticleIndex});
                }

                // Calculate the new COM and total mass by including the new particle
                float newParticleMass = data.particleMass[particleIndex];
                float newParticleX = data.particleX[particleIndex];
                float newParticleY = data.particleY[particleIndex];
                float totalMassBeforeInsert = data.nodeTotalMass[nodeIndex];

                // Update the node's total mass
                data.nodeTotalMass[nodeIndex] += newParticleMass;

                // Update the node's COM using the formula: newCOM = (oldCOM * oldMass + newParticlePos * newParticleMass) / newTotalMass
                data.nodeCOM_X[nodeIndex] =
                        (data.nodeCOM_X[nodeIndex] * totalMassBeforeInsert + newParticleX * newParticleMass) /
                        data.nodeTotalMass[nodeIndex];
                data.nodeCOM_Y[nodeIndex] =
                        (data.nodeCOM_Y[nodeIndex] * totalMassBeforeInsert + newParticleY * newParticleMass) /
                        data.nodeTotalMass[nodeIndex];
            }

            // Determine the quadrant for the new particle and create a new child node if necessary
            int childIndex = 0;
            if (data.particleX[particleIndex] > data.nodeX[nodeIndex] + halfWidth)
                childIndex |= 1;

            if (data.particleY[particleIndex] > data.nodeY[nodeIndex] + halfHeight)
                childIndex |= 2;

            if (data.nodeChildren[nodeIndex][childIndex] == NULL_INDEX)
            {
                float childX = data.nodeX[nodeIndex] + (childIndex & 1 ? halfWidth : 0);
                float childY = data.nodeY[nodeIndex] + (childIndex & 2 ? halfHeight : 0);

                data.nodeChildren[nodeIndex][childIndex] = createNode(childX, childY, halfWidth, halfHeight, data);
            }

            // Push the new particle to the appropriate child node
            stack.push({data.nodeChildren[nodeIndex][childIndex], particleIndex});
        }
    }
}

void Octree::buildTree(const SimulationData &data)
{
    nodeCount = 0;
    int rootNodeIndex = createNode(-32768.0f, -32768.0f, 65536.0f, 65536.0f, data);

    unsigned int mortonIndex[MAX_PARTICLES];

    std::iota(data.idxSorted, data.idxSorted + MAX_PARTICLES, 0);

    for (int i = 0; i < MAX_PARTICLES; ++i)
    {
        mortonIndex[i] = morton2D(data.particleX[i], data.particleY[i]);
    }

    std::sort(data.idxSorted, data.idxSorted + MAX_PARTICLES,
              [&mortonIndex](int i1, int i2) { return mortonIndex[i1] < mortonIndex[i2]; });

#pragma omp parallel for shared(rootNodeIndex, data)\
                        default(none) \
                        schedule(static, 100)
    for (int i = 0; i < MAX_PARTICLES; i++)
    {
        int sortedParticleIndex = static_cast<int>(data.idxSorted[i]);
        if (data.particleMass[sortedParticleIndex] > 0)
        {
            insertParticleToNode(rootNodeIndex, sortedParticleIndex, data);
        }
    }
}