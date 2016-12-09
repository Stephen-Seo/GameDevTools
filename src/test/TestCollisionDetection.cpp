
#include "gtest/gtest.h"

#include <GDT/CollisionDetection.hpp>

TEST(CollisionDetection, isWithinPolygon)
{
    {
        // 2 by 2 diamond centered at 1,1
        float arrayOfPairs[4][2] =
        {
            {1.0f, 0.0f},
            {2.0f, 1.0f},
            {1.0f, 2.0f},
            {0.0f, 1.0f}
        };
        unsigned int size = 4;

        // center
        EXPECT_TRUE(GDT::isWithinPolygon(arrayOfPairs, size, 1.0f, 1.0f));

        // just within sides
        EXPECT_TRUE(GDT::isWithinPolygon(arrayOfPairs, size, 0.6f, 0.6f));
        EXPECT_TRUE(GDT::isWithinPolygon(arrayOfPairs, size, 0.6f, 1.4f));
        EXPECT_TRUE(GDT::isWithinPolygon(arrayOfPairs, size, 1.4f, 0.6f));
        EXPECT_TRUE(GDT::isWithinPolygon(arrayOfPairs, size, 1.4f, 1.4f));

        // outside corners
        EXPECT_FALSE(GDT::isWithinPolygon(arrayOfPairs, size, 0.0f, 0.0f));
        EXPECT_FALSE(GDT::isWithinPolygon(arrayOfPairs, size, 2.0f, 0.0f));
        EXPECT_FALSE(GDT::isWithinPolygon(arrayOfPairs, size, 2.0f, 2.0f));
        EXPECT_FALSE(GDT::isWithinPolygon(arrayOfPairs, size, 0.0f, 2.0f));
    }

    {
        // 2 by 2 square centered at -2,-2
        float arrayOfPairs[4][2] =
        {
            {-3.0f, -3.0f},
            {-1.0f, -3.0f},
            {-1.0f, -1.0f},
            {-3.0f, -1.0f}
        };
        unsigned int size = 4;

        // center
        EXPECT_TRUE(GDT::isWithinPolygon(arrayOfPairs, size, -2.0f, -2.0f));

        // just within sides
        EXPECT_TRUE(GDT::isWithinPolygon(arrayOfPairs, size, -2.0f, -2.9f));
        EXPECT_TRUE(GDT::isWithinPolygon(arrayOfPairs, size, -2.0f, -1.1f));
        EXPECT_TRUE(GDT::isWithinPolygon(arrayOfPairs, size, -2.9f, -2.0f));
        EXPECT_TRUE(GDT::isWithinPolygon(arrayOfPairs, size, -1.1f, -2.0f));

        // outside next to corners
        EXPECT_FALSE(GDT::isWithinPolygon(arrayOfPairs, size, -4.0f, -3.0f));
        EXPECT_FALSE(GDT::isWithinPolygon(arrayOfPairs, size, -3.0f, -4.0f));

        EXPECT_FALSE(GDT::isWithinPolygon(arrayOfPairs, size, -1.0f, -4.0f));
        EXPECT_FALSE(GDT::isWithinPolygon(arrayOfPairs, size, 0.0f, -3.0f));

        EXPECT_FALSE(GDT::isWithinPolygon(arrayOfPairs, size, -1.0f, 0.0f));
        EXPECT_FALSE(GDT::isWithinPolygon(arrayOfPairs, size, 0.0f, -1.0f));

        EXPECT_FALSE(GDT::isWithinPolygon(arrayOfPairs, size, -3.0f, 0.0f));
        EXPECT_FALSE(GDT::isWithinPolygon(arrayOfPairs, size, -4.0f, -1.0f));
    }

    {
        // 2 by 2 diamond centered at 1,1
        float arrayOfPairs[8] =
        {
            1.0f, 0.0f,
            2.0f, 1.0f,
            1.0f, 2.0f,
            0.0f, 1.0f
        };
        unsigned int arraySize = 8;

        // center
        EXPECT_TRUE(GDT::isWithinPolygon(arrayOfPairs, arraySize, 1.0f, 1.0f));

        // just within sides
        EXPECT_TRUE(GDT::isWithinPolygon(arrayOfPairs, arraySize, 0.6f, 0.6f));
        EXPECT_TRUE(GDT::isWithinPolygon(arrayOfPairs, arraySize, 0.6f, 1.4f));
        EXPECT_TRUE(GDT::isWithinPolygon(arrayOfPairs, arraySize, 1.4f, 0.6f));
        EXPECT_TRUE(GDT::isWithinPolygon(arrayOfPairs, arraySize, 1.4f, 1.4f));

        // outside corners
        EXPECT_FALSE(GDT::isWithinPolygon(arrayOfPairs, arraySize, 0.0f, 0.0f));
        EXPECT_FALSE(GDT::isWithinPolygon(arrayOfPairs, arraySize, 2.0f, 0.0f));
        EXPECT_FALSE(GDT::isWithinPolygon(arrayOfPairs, arraySize, 2.0f, 2.0f));
        EXPECT_FALSE(GDT::isWithinPolygon(arrayOfPairs, arraySize, 0.0f, 2.0f));
    }

    {
        // 2 by 2 square centered at -2,-2
        float arrayOfPairs[8] =
        {
            -3.0f, -3.0f,
            -1.0f, -3.0f,
            -1.0f, -1.0f,
            -3.0f, -1.0f
        };
        unsigned int arraySize = 8;

        // center
        EXPECT_TRUE(GDT::isWithinPolygon(arrayOfPairs, arraySize, -2.0f, -2.0f));

        // just within sides
        EXPECT_TRUE(GDT::isWithinPolygon(arrayOfPairs, arraySize, -2.0f, -2.9f));
        EXPECT_TRUE(GDT::isWithinPolygon(arrayOfPairs, arraySize, -2.0f, -1.1f));
        EXPECT_TRUE(GDT::isWithinPolygon(arrayOfPairs, arraySize, -2.9f, -2.0f));
        EXPECT_TRUE(GDT::isWithinPolygon(arrayOfPairs, arraySize, -1.1f, -2.0f));

        // outside next to corners
        EXPECT_FALSE(GDT::isWithinPolygon(arrayOfPairs, arraySize, -4.0f, -3.0f));
        EXPECT_FALSE(GDT::isWithinPolygon(arrayOfPairs, arraySize, -3.0f, -4.0f));

        EXPECT_FALSE(GDT::isWithinPolygon(arrayOfPairs, arraySize, -1.0f, -4.0f));
        EXPECT_FALSE(GDT::isWithinPolygon(arrayOfPairs, arraySize, 0.0f, -3.0f));

        EXPECT_FALSE(GDT::isWithinPolygon(arrayOfPairs, arraySize, -1.0f, 0.0f));
        EXPECT_FALSE(GDT::isWithinPolygon(arrayOfPairs, arraySize, 0.0f, -1.0f));

        EXPECT_FALSE(GDT::isWithinPolygon(arrayOfPairs, arraySize, -3.0f, 0.0f));
        EXPECT_FALSE(GDT::isWithinPolygon(arrayOfPairs, arraySize, -4.0f, -1.0f));
    }
}

