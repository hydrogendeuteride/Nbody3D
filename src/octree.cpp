#include <cmath>
#include <stack>
#include <numeric>
#include <iostream>
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

static int findJustBigNumber(const uint64_t *arr, int left, int right, uint64_t number)
{
    while (left <= right)
    {
        int mid = left + (right - left) / 2;

        if (arr[mid] >= number)
        {
            if (arr[mid] == number)
                return mid;
            right = mid - 1;
        }
        else
        {
            left = mid + 1;
        }
    }
    return left < right ? left : -1;
}

static int findJustSmallNumber(const uint64_t *arr, int left, int right, uint64_t number)
{
    while (left <= right)
    {
        int mid = left + (right - left) / 2;

        if (arr[mid] <= number)
        {
            if (arr[mid] == number)
                return mid;
            left = mid + 1;
        }
        else
        {
            right = mid - 1;
        }
    }
    return right >= 0 ? right : -1;
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

int Octree::generateNode(SimulationData &data, int numParticles)    //needs sorted morton code array
{
    int first;
    int last = 0;

    int rootIndex = -1;

    int depth = 0;
    int bitShift = 3;
    while ((float) depth < std::ceil(log2f(65536)) + 1)
    {
        //can be parallelized using depth(bitshift level)
        first = 0;

        while (last < numParticles)
        {
            last = first + 1;

            while (last < numParticles &&
                   (data.mortonIndex[first] >> bitShift) == (data.mortonIndex[last] >> bitShift))
            {

                last++; //Move when meets same depth of node
            }

            if ((first != (last - 1)) &&
                ((data.mortonIndex[first] >> bitShift) == (data.mortonIndex[last - 1] >> bitShift)))
            {
                //If particle is alone in one big node, prevent generating every node in every depth iteration
                float x, y, z;
                morton3DInverse(data.mortonIndex[first] >> (bitShift) << (bitShift), x, y, z);
                float size = powf(2.0f, (float) (depth));

                int nodeIndex = -1;//createNode(x, y, z, size, size, size, data);

                uint64_t prevMorton = UINT64_MAX;
                for (int i = first; i < last; ++i)
                {
                    int childIndex = (data.mortonIndex[i] >> (3 * depth)) & 7;

                    float childX = x + (childIndex & 1) * size;
                    float childY = y + ((childIndex >> 1) & 1) * size;
                    float childZ = z + ((childIndex >> 2) & 1) * size;

                    if (((data.mortonIndex[i] >> (3 * depth)) & 7) == prevMorton)
                        continue;

                    prevMorton = (data.mortonIndex[i] >> (3 * depth)) & 7;

                    nodeIndex = createNode(childX, childY, childZ, size, size, size, data);
                    std::cout << nodeIndex << " " << first << " " << last << " " << size << " " << childX << " "
                              << childY << " " << childZ << " " << depth << " " << std::endl;
                }

                rootIndex = nodeIndex;
            }
            first = last;
        }

        depth++;
        bitShift += 3;
        last = 0;
    }

    return rootIndex;
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

void Octree::nodeCOMInit(SimulationData &data)
{
    for (int i = 0; i < MAX_NODES; ++i) //Can be parallelized
    {
        float x = data.nodeX[i];
        float y = data.nodeY[i];
        float z = data.nodeZ[i];

        float size = data.nodeWidth[i];

        uint64_t first = morton3D(x, y, z);
        uint64_t last = morton3D(x + size, y + size, z + size);

        int f = findJustBigNumber(data.mortonIndex, 0, MAX_PARTICLES, first);
        int l = findJustSmallNumber(data.mortonIndex, 0, MAX_PARTICLES, last);

        for (int j = f; j <= l; ++j)
        {
            if (data.particleMass[j] > 0)
            {
                float totalMassBeforeInsert = data.nodeTotalMass[i];
                float newParticleMass = data.particleMass[j];

                data.nodeTotalMass[i] += newParticleMass;
                data.nodeCOM_X[i] = (data.nodeCOM_X[i] * totalMassBeforeInsert +
                                     data.particleX[j] * newParticleMass) / data.nodeTotalMass[i];
                data.nodeCOM_Y[i] = (data.nodeCOM_Y[i] * totalMassBeforeInsert +
                                     data.particleY[j] * newParticleMass) / data.nodeTotalMass[i];
                data.nodeCOM_Z[i] = (data.nodeCOM_Z[i] * totalMassBeforeInsert +
                                     data.particleZ[j] * newParticleMass) / data.nodeTotalMass[i];
            }
        }
    }
}

int Octree::buildTree(SimulationData &data, int numParticles)
{
    nodeCount = 0;

    std::iota(data.idxSorted, data.idxSorted + numParticles, 0);

    for (int i = 0; i < numParticles; ++i)
    {
        data.mortonIndex[i] = morton3D(data.particleX[i], data.particleY[i], data.particleZ[i]);
    }

    std::sort(data.idxSorted, data.idxSorted + numParticles,
              [&data](int i1, int i2) { return data.mortonIndex[i1] < data.mortonIndex[i2]; });

    std::sort(data.mortonIndex, data.mortonIndex + numParticles);

    int root = generateNode(data, numParticles);
    nodeCOMInit(data);

    return root;
}