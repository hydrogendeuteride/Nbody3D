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

static int findJustBigNumber(const uint64_t *arr, int n, uint64_t target)
{
    int low = 0, high = n - 1, result = -1;
    while (low <= high)
    {
        int mid = low + (high - low) / 2;

        if (arr[mid] >= target)
        {
            result = mid;
            high = mid - 1;
        }
        else
        {
            low = mid + 1;
        }
    }
    return result;
}

static int findExactNumber(const uint64_t *arr, int n, uint64_t target)
{
    int low = 0, high = n - 1;
    while (low <= high)
    {
        int mid = low + (high - low) / 2;

        if (arr[mid] == target)
        {
            return mid;
        }
        else if (arr[mid] < target)
        {
            low = mid + 1;
        }
        else
        {
            high = mid - 1;
        }
    }
    return -1;
}


static int findJustSmallNumber(const uint64_t *arr, int n, uint64_t target)
{
    int low = 0, high = n - 1, result = -1;
    while (low <= high)
    {
        int mid = low + (high - low) / 2;

        if (arr[mid] <= target)
        {
            result = mid;
            low = mid + 1;
        }
        else
        {
            high = mid - 1;
        }
    }
    return result;
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

void Octree::calcNumNodes(SimulationData &data, int numParticles)
{
    uint64_t prevCode = 0;
    int pointer = 0;

    int maxLength = ((64 - __builtin_clzll(data.mortonIndex[numParticles - 1])) / 3) * 3;

    for (int j = maxLength; j > 0; j-=3)
    {
        for (int i = 0; i < numParticles; ++i)
        {
            if ((data.mortonIndex[i] >> j) != prevCode)
            {
                numNodeArray[pointer++] = pointer;
                prevCode = data.mortonIndex[i] >> j;
            }
        }
    }
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
                    int childIndex = (int) (data.mortonIndex[i] >> (3 * depth)) & 7;

                    float childX = x + (float) (childIndex & 1) * size;
                    float childY = y + (float) ((childIndex >> 1) & 1) * size;
                    float childZ = z + (float) ((childIndex >> 2) & 1) * size;

                    if (((data.mortonIndex[i] >> (3 * depth)) & 7) == prevMorton)
                        continue;

                    prevMorton = (data.mortonIndex[i] >> (3 * depth)) & 7;

                    uint64_t nodeMortonCode  = (data.mortonIndex[i]) | 1 << (64 - __builtin_clzll(data.mortonIndex[i])) >> (3 * depth);

                    nodeIndex = createNode(childX, childY, childZ, size, size, size,
                                           nodeMortonCode, data);
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
                       uint64_t nodeMortoncode, const SimulationData &data)
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

    data.nodeMortonIdx[index] = nodeMortoncode;

    for (int i = 0; i < OCT_CHILD; i++)
    {
        data.nodeChildren[index][i] = NULL_INDEX;
    }
    return index;
}

bool isParticleInNode1(float pX, float pY, float pZ, float nX, float nY, float nZ, float nW, float nH, float nD)
{
    return pX >= nX && pX <= nX + nW &&
           pY >= nY && pY <= nY + nH &&
           pZ >= nZ && pZ <= nZ + nD;
}

void Octree::nodeCOMInit(SimulationData &data, int numParticles) const
{
    for (int i = 0; i < nodeCount; ++i) //Can be parallelized
    {
        float x = data.nodeX[i];
        float y = data.nodeY[i];
        float z = data.nodeZ[i];

        float size = data.nodeWidth[i];

        std::cout << std::endl;
        std::cout << x << " " << y << " " << z << " " << size << std::endl;

        uint64_t first = morton3D(x, y, z);
        uint64_t last = morton3D(x + size, y + size, z + size);

        int f = findJustBigNumber(data.mortonIndex, numParticles, first);
        int l = findJustSmallNumber(data.mortonIndex, numParticles, last);

        std::cout << first << " " << last << " " << f << " " << l;

        for (int j = f; j <= l; ++j)
        {
            if (data.particleMass[j] > 0
                && isParticleInNode1(data.particleX[j], data.particleY[j], data.particleZ[j],
                                     data.nodeX[i], data.nodeY[i], data.nodeZ[i],
                                     data.nodeWidth[i], data.nodeHeight[i], data.nodeDepth[i]))
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

void Octree::nodeChildrenInit(SimulationData &data)  //needs sorted node morton code
{
    for (int i = 0; i < nodeCount; ++i)
    {
        unsigned int depth = static_cast<unsigned int>( log2f(65536.0f / data.nodeWidth[i]));

        uint64_t childMortonCode;
        for (int j = 0; j < OCT_CHILD; ++j)
        {
            childMortonCode = data.nodeMortonIdx[i] |
                    (j >> (3 * (int) (log2f(65536.0f) - (float) depth)) << (3 * (int) (log2f(65536.0f) - (float) depth)));
            data.nodeChildren[i][j] = findExactNumber(data.nodeMortonIdx, nodeCount, childMortonCode);
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

    for (int i = 0; i < numParticles; ++i)
    {
        std::cout << data.particleX[data.idxSorted[i]] << " " << data.particleY[data.idxSorted[i]] <<
                  " " << data.particleZ[data.idxSorted[i]] << " " << data.mortonIndex[i] << " \n";
    }
    std::cout << std::endl;

    int root = generateNode(data, numParticles);
    nodeCOMInit(data, numParticles);

    std::sort(data.nodeMortonIdx, data.nodeMortonIdx + nodeCount);

    for (int i = 0; i < nodeCount; ++i)
    {
        std::cout << data.nodeX[i] << " " << data.nodeY[i] <<
                  " " << data.nodeZ[i] << " " << data.nodeWidth[i] << " \n";
    }
    nodeChildrenInit(data);

    return root;
}