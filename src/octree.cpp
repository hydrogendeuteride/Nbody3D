#include <algorithm>
#include <cmath>
#include <stack>
#include <numeric>
#include <cmath>
#include "octree.h"
#include "omp.h"

template<unsigned int p>
int constexpr IntPower(const int x)
{
    if constexpr (p == 0) return 1;
    if constexpr (p == 1) return x;

    int tmp = IntPower<p / 2>(x);
    if constexpr ((p % 2) == 0)
    { return tmp * tmp; }
    else
    { return x * tmp * tmp; }
}

static constexpr uint64_t expandBits(uint64_t v)
{
    v = (v | v << 32) & 0x1f00000000ffff;
    v = (v | v << 16) & 0x1f0000ff0000ff;
    v = (v | v << 8) & 0x100f00f00f00f00f;
    v = (v | v << 4) & 0x10c30c30c30c30c3;
    v = (v | v << 2) & 0x1249249249249249;

    return v;
}

static constexpr uint64_t morton3D(float x, float y, float z)
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

static constexpr uint64_t compactBits(uint64_t v)
{
    v &= 0x1249249249249249;
    v = (v ^ (v >> 2)) & 0x10c30c30c30c30c3;
    v = (v ^ (v >> 4)) & 0x100f00f00f00f00f;
    v = (v ^ (v >> 8)) & 0x1f0000ff0000ff;
    v = (v ^ (v >> 16)) & 0x1f00000000ffff;
    v = (v ^ (v >> 32)) & 0xffff;
    return v;
}

static constexpr void morton3DInverse(uint64_t code, float &x, float &y, float &z)
{
    uint64_t xx = compactBits(code);
    uint64_t yy = compactBits(code >> 1);
    uint64_t zz = compactBits(code >> 2);

    x = static_cast<float>(xx) - 32768.0f;
    y = static_cast<float>(yy) - 32768.0f;
    z = static_cast<float>(zz) - 32768.0f;
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

void Octree::generateNode(SimulationData &data, int first, int last)
{
    int depth = 0;
    int bitShift = 3;
    while ((float) depth < std::ceil(log2f(MAX_PARTICLES) / 3.0f))
    {
        //can be parallelized using depth(bitshift level)
        first = 0;

        while (last < MAX_PARTICLES)
        {
            last = first + 1;
            int childs[8];
            int idx = 0;

            while (last < MAX_PARTICLES && (data.idxSorted[first] >> bitShift) == (data.idxSorted[last] >> bitShift))
            {
                childs[idx] = first;
                idx++;
                last++; // one more child node to node
            }

            //else, generate node and move right
            float x, y, z;
            morton3DInverse(data.idxSorted[first], x, y, z);
            float size = powf(2.0f, (float) depth);
            createNode(x, y, z, size, size, size, data);
            //todo: edit createNode to save node child information.
            first = last;
        }

        depth++;
        bitShift += 3;
    }
}

int Octree::createNode(float x, float y, float z, float width, float height, float depth,
                       const SimulationData &data)
{
    int index;
#pragma omp atomic capture
    index = nodeCount++;

    data.nodeX[index] = x;
    data.nodeY[index] = y;
    data.nodeZ[index] = z;

    data.nodeWidth[index] = width;
    data.nodeHeight[index] = height;
    data.nodeDepth[index] = depth;

    data.nodeParticleIndex[index] = NULL_INDEX;
    for (int i = 0; i < OCT_CHILD; i++)
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

                    if (data.nodeChildren[nodeIndex][childIndex] == NULL_INDEX)
                    {
                        float childX = data.nodeX[nodeIndex] + (childIndex & 1 ? halfWidth : 0);
                        float childY = data.nodeY[nodeIndex] + (childIndex & 2 ? halfHeight : 0);
                        float childZ = data.nodeZ[nodeIndex] + (childIndex & 4 ? halfDepth : 0);
                        data.nodeChildren[nodeIndex][childIndex] = createNode(childX, childY, childZ,
                                                                              halfWidth, halfHeight, halfDepth,
                                                                              data);
                    }
                    stack.push({data.nodeChildren[nodeIndex][childIndex], existingParticleIndex});
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

            if (data.nodeChildren[nodeIndex][childIndex] == NULL_INDEX)
            {
                float childX = data.nodeX[nodeIndex] + (childIndex & 1 ? halfWidth : 0);
                float childY = data.nodeY[nodeIndex] + (childIndex & 2 ? halfHeight : 0);
                float childZ = data.nodeZ[nodeIndex] + (childIndex & 4 ? halfDepth : 0);

                data.nodeChildren[nodeIndex][childIndex] = createNode(childX, childY, childZ,
                                                                      halfWidth, halfHeight, halfDepth,
                                                                      data);
            }

            // Push the new particle to the appropriate child node
            stack.push({data.nodeChildren[nodeIndex][childIndex], particleIndex});
        }
    }
}

void Octree::buildTree(const SimulationData &data)
{
    nodeCount = 0;
    int rootNodeIndex = createNode(-32768.0f, -32768.0f, -32768.0f,
                                   65536.0f, 65536.0f, 65536.0f, data);

    unsigned int mortonIndex[MAX_PARTICLES];

    std::iota(data.idxSorted, data.idxSorted + MAX_PARTICLES, 0);

    for (int i = 0; i < MAX_PARTICLES; ++i)
    {
        mortonIndex[i] = morton3D(data.particleX[i], data.particleY[i], data.particleZ[i]);
    }

    std::sort(data.idxSorted, data.idxSorted + MAX_PARTICLES,
              [&mortonIndex](int i1, int i2) { return mortonIndex[i1] < mortonIndex[i2]; });

#pragma omp parallel for shared(rootNodeIndex, data)\
                        default(none)
    for (int i = 0; i < MAX_PARTICLES; i++)
    {
        int sortedParticleIndex = static_cast<int>(data.idxSorted[i]);
        if (data.particleMass[sortedParticleIndex] > 0)
        {
            insertParticleToNode(rootNodeIndex, sortedParticleIndex, data);
        }
    }
}