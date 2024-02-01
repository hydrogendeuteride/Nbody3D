#include <gtest/gtest.h>
#include "svotree.h"

#define UNIT_TEST

TEST(SVOctree_Test, TestSolve)
{
    SVOctree tree;

    ParticleData data;

    std::vector<uint64_t> mortonCodes = {0x000, 0x001, 0x002, 0x003, 0x010, 0x011, 0x012, 0x013};
    data.mortonCode = mortonCodes.data();

    auto range1 = tree.determineRange(data, 8, 0);

    EXPECT_EQ(range1.x, 0);
    EXPECT_EQ(range1.y, 7);

    std::vector<uint64_t> mortonCodes2 =
            {0b1000, 0b1001, 0b1010, 0b1011, 0b1100, 0b1101, 0b1110, 0b1111};
    data.mortonCode = mortonCodes2.data();

    auto range2 = tree.determineRange(data, 8, 0);

    EXPECT_EQ(range2.x, 0);
    EXPECT_EQ(range2.y, 7);

    std::vector<uint64_t> mortonCodes3 =
            {0b000000, 0b000001, 0b000010, 0b000011, 0b000100, 0b000101, 0b000110, 0b000111,
             0b001000, 0b001001, 0b001010, 0b001011, 0b001100, 0b001101, 0b000110, 0b001111
            };

    data.mortonCode = mortonCodes3.data();

    auto range3 = tree.determineRange(data, 16, 0);

    EXPECT_EQ(range3.x, 0);
    EXPECT_EQ(range3.y, 15);

    auto range4 = tree.determineRange(data, 16, 1);

    EXPECT_EQ(range4.x, 0);
    EXPECT_EQ(range4.y, 7);
}