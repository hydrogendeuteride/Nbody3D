#include "svotree.h"
#include <stack>
#include <numeric>

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

pair SVOctree::determineRange(ParticleData &data, int numParticles, int idx)
{
    if (idx >= numParticles - 1)
    {
        return {0, 0};
    }

    auto delta = [&](int i, int j) -> int {
        if (j < 0 || j > numParticles - 1)
            return -1;
        return __builtin_clz(data.mortonCode[i] ^ data.mortonCode[j]) / 3; //for 64 bit, 64 -1 = 63
    };

    int d = delta(idx, idx + 1) > delta(idx, idx - 1) ? 1 : -1;
    int deltaMin = delta(idx, idx - d);
    int lmax = 2;

    while (idx + lmax * d < numParticles && idx + lmax * d >= 0 &&
           delta(idx, idx + lmax * d) > deltaMin)
    {
        lmax *= 2;
    }

    int l = 0;
    for (int t = lmax / 2; t >= 1; t /= 2)
    {
        if (idx + (l + t) * d < numParticles && idx + (l + t) * d >= 0 &&
            delta(idx, idx + (l + t) * d) > deltaMin)
        {
            l += t;
        }
    }

    return {std::min(idx, idx + l * d), std::max(idx, idx + l * d)};
}

int SVOctree::findSplit(ParticleData &data, int first, int last)
{
    uint64_t firstCode = data.mortonCode[first];
    uint64_t lastCode = data.mortonCode[last];

    if (firstCode == lastCode)
        return (first + last) >> 1;

    int commonPrefix = __builtin_clz(firstCode ^ lastCode);

    int split = first;
    int step = last - first;

    do
    {
        step = (step + 1) >> 1;
        int newSplit = split + step;

        if (newSplit < last)
        {
            uint64_t splitCode = data.mortonCode[newSplit];
            int splitPrefix = __builtin_clz(firstCode ^ splitCode);
            if (splitPrefix > commonPrefix)
                split = newSplit;
        }

    } while (step > 1);

    return split;
}

int SVOctree::generateNode(ParticleData &data, NodeData &nodeData, int numParticles)
{
    for (int i = 0; i < numParticles; ++i)
    {
        nodeData.leafNodes[i] = data.idxSorted[i];
    }

    for (int i = 0; i < numParticles - 1; ++i)
    {
        pair range = determineRange(data, numParticles, i);

        int first = range.x;
        int last = range.y;

        int split = findSplit(data, first, last);


    }
}