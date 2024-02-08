#include <algorithm>
#include <stack>
#include <numeric>
#include <cmath>
#include "octree.h"
#include "omp.h"

uint64_t Octree::expandBits(uint64_t v)
{
    v = (v | v << 32) & 0x1f00000000ffff;
    v = (v | v << 16) & 0x1f0000ff0000ff;
    v = (v | v << 8) & 0x100f00f00f00f00f;
    v = (v | v << 4) & 0x10c30c30c30c30c3;
    v = (v | v << 2) & 0x1249249249249249;

    return v;
}

uint64_t Octree::morton3D(float x, float y, float z)
{
    x += 32768.0f;
    y += 32768.0f;
    z += 32768.0f;

    auto scaledX = (unsigned int) x;
    auto scaledY = (unsigned int) y;
    auto scaledZ = (unsigned int) z;

    scaledX = std::max(0U, std::min(scaledX, 65536U));
    scaledY = std::max(0U, std::min(scaledY, 65536U));
    scaledZ = std::max(0U, std::min(scaledZ, 65536U));

    uint64_t xx = expandBits(scaledX);
    uint64_t yy = expandBits(scaledY);
    uint64_t zz = expandBits(scaledZ);

    return xx | (yy << 1) | (zz << 2);
}

bool Octree::noChildren(const SimulationData &data, int nodeIndex)
{
    for (int i = 0; i < OCT_CHILD; ++i)
    {
        if (data.nodeChildren[nodeIndex][i] != NULL_INDEX)
            return false;
    }

    return true;
}

int Octree::createNode(float x, float y, float z, float size, uint64_t mortonCode,
                       const SimulationData &data)
{
    int index;
#pragma omp atomic capture
    index = nodeCount++;

    data.nodeX[index] = x;
    data.nodeY[index] = y;
    data.nodeZ[index] = z;

    data.nodeWidth[index] = size;
    data.nodeHeight[index] = size;
    data.nodeDepth[index] = size;

    data.nodeMortonCode[index] = mortonCode;

    data.nodeParticleIndex[index] = NULL_INDEX;
    for (int i = 0; i < OCT_CHILD; i++)
    {
        data.nodeChildren[index][i] = NULL_INDEX;
    }
    return index;
}

void Octree::insertParticleToNode(int nodeIndex, int particleIndex, int maxDepth, const SimulationData &data)
{
    std::stack<pair> stack;
    stack.push({nodeIndex, particleIndex, 0});

    int depth;

    uint64_t mortonCode = 1;

    while (!stack.empty())
    {
        auto top = stack.top();
        stack.pop();
        nodeIndex = top.node;
        particleIndex = top.particle;
        depth = top.depth;

        // If the node is a leaf node with no children, simply insert the particle
        if (data.nodeParticleIndex[nodeIndex] == NULL_INDEX && noChildren(data, nodeIndex))
        {
            data.nodeParticleIndex[nodeIndex] = particleIndex;

            data.nodeTotalMass[nodeIndex] = data.particleMass[particleIndex];
            data.nodeCOM_X[nodeIndex] = data.particleX[particleIndex];
            data.nodeCOM_Y[nodeIndex] = data.particleY[particleIndex];
            data.nodeCOM_Z[nodeIndex] = data.particleZ[particleIndex];

        }
        else
        {
            float halfWidth = data.nodeWidth[nodeIndex] / 2.0f;
            float halfHeight = data.nodeHeight[nodeIndex] / 2.0f;
            float halfDepth = data.nodeDepth[nodeIndex] / 2.0f;
            // If the node already contains a particle or has children, we need to update the COM and mass
            // Generate new node for existing particle and push them to stack
            if (data.nodeParticleIndex[nodeIndex] != NULL_INDEX || !noChildren(data, nodeIndex))
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
                    if (data.particleZ[existingParticleIndex] > data.nodeZ[nodeIndex] + halfDepth)
                    {
                        childIndex |= 4;
                    }

                    if (data.nodeChildren[nodeIndex][childIndex] == NULL_INDEX && depth < maxDepth)
                    {
                        float childX = data.nodeX[nodeIndex] + (childIndex & 1 ? halfWidth : 0);
                        float childY = data.nodeY[nodeIndex] + (childIndex & 2 ? halfHeight : 0);
                        float childZ = data.nodeZ[nodeIndex] + (childIndex & 4 ? halfDepth : 0);

                        mortonCode = (mortonCode << 3 | childIndex);

                        data.nodeChildren[nodeIndex][childIndex] = createNode(childX, childY, childZ,
                                                                              halfWidth, mortonCode,
                                                                              data);
                    }
                    if (depth < maxDepth)
                        stack.push({data.nodeChildren[nodeIndex][childIndex], existingParticleIndex, ++depth});
                }
                // Calculate the new COM and total mass by including the new particle
                float newParticleMass = data.particleMass[particleIndex];
                float newParticleX = data.particleX[particleIndex];
                float newParticleY = data.particleY[particleIndex];
                float newParticleZ = data.particleZ[particleIndex];
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
                data.nodeCOM_Z[nodeIndex] =
                        (data.nodeCOM_Z[nodeIndex] * totalMassBeforeInsert + newParticleZ * newParticleMass) /
                        data.nodeTotalMass[nodeIndex];
            }
            // Determine the quadrant for the new particle and create a new child node if necessary
            int childIndex = 0;
            if (data.particleX[particleIndex] > data.nodeX[nodeIndex] + halfWidth)
                childIndex |= 1;

            if (data.particleY[particleIndex] > data.nodeY[nodeIndex] + halfHeight)
                childIndex |= 2;

            if (data.particleZ[particleIndex] > data.nodeZ[nodeIndex] + halfDepth)
                childIndex |= 4;

            if (data.nodeChildren[nodeIndex][childIndex] == NULL_INDEX && depth < maxDepth)
            {
                float childX = data.nodeX[nodeIndex] + (childIndex & 1 ? halfWidth : 0);
                float childY = data.nodeY[nodeIndex] + (childIndex & 2 ? halfHeight : 0);
                float childZ = data.nodeZ[nodeIndex] + (childIndex & 4 ? halfDepth : 0);

                data.nodeChildren[nodeIndex][childIndex] = createNode(childX, childY, childZ,
                                                                      halfWidth, mortonCode,
                                                                      data);
            }

            // Push the new particle to the appropriate child node
            if (depth < maxDepth)
                stack.push({data.nodeChildren[nodeIndex][childIndex], particleIndex, ++depth});
        }
    }
}

void Octree::makeLeafNode(SimulationData& data)
{
    for (int i = 0; i < nodeCount; ++i)
    {
        if (noChildren(data, i))
        {
            int childIndex = 0;
            int particleIdx = data.nodeParticleIndex[i];
            float halfWidth = data.nodeWidth[i] / 2;

            if (data.particleX[particleIdx] > data.nodeX[i] + halfWidth)
                childIndex |= 1;

            if (data.particleY[particleIdx] > data.nodeY[i] + halfWidth)
                childIndex |= 2;

            if (data.particleZ[particleIdx] > data.nodeZ[i] + halfWidth)
                childIndex |= 4;

            float leafX = std::floor(data.particleX[particleIdx]);
            float leafY = std::floor(data.particleY[particleIdx]);
            float leafZ = std::floor(data.particleZ[particleIdx]);

            data.nodeChildren[i][childIndex] = createNode(leafX, leafY, leafZ,
                                                          1.0f < data.nodeWidth[i] ? 1.0f : data.nodeWidth[i],
                                                          morton3D(leafX, leafY, leafZ), data);
        }
    }
}

void Octree::buildTree(const SimulationData &data)
{
    nodeCount = 0;
    int rootNodeIndex = createNode(-32768.0f, -32768.0f, -32768.0f,
                                   65536.0f, 1, data);

    unsigned int mortonIndex[MAX_PARTICLES];

    std::iota(data.idxSorted, data.idxSorted + MAX_PARTICLES, 0);

    for (int i = 0; i < MAX_PARTICLES; ++i)
    {
        mortonIndex[i] = morton3D(data.particleX[i], data.particleY[i], data.particleZ[i]);
    }

    std::sort(data.idxSorted, data.idxSorted + MAX_PARTICLES,
              [&mortonIndex](int i1, int i2) { return mortonIndex[i1] < mortonIndex[i2]; });

    for (int i = 0; i < MAX_PARTICLES; i++)
    {
        int sortedParticleIndex = static_cast<int>(data.idxSorted[i]);
        if (data.particleMass[sortedParticleIndex] > 0)
        {
            insertParticleToNode(rootNodeIndex, sortedParticleIndex, 100, data);
        }
    }
}