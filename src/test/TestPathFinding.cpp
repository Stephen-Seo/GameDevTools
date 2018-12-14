#include "gtest/gtest.h"

#include <GDT/PathFinding.hpp>

TEST(PathFinding, SimpleWall)
{
    char wall[30] = {
        0, 0, 1, 0, 0,
        0, 0, 1, 0, 0,
        0, 0, 1, 0, 0,
        0, 0, 1, 0, 0,
        0, 0, 0, 0, 0,
        0, 0, 0, 0, 0
    };
    auto result = GDT::findPath<char, int>(
        wall,
        11, // start
        3, // end
        5, // width
        30, // size
        [] (const char* array, const int& pos) {
            return array[pos] == 1;
        });

    EXPECT_EQ(result.at(11), 16);
    EXPECT_EQ(result.at(16), 21);
    EXPECT_EQ(result.at(21), 22);
    EXPECT_EQ(result.at(22), 23);
    EXPECT_EQ(result.at(23), 18);
    EXPECT_EQ(result.at(18), 13);
    EXPECT_EQ(result.at(13), 8);
    EXPECT_EQ(result.at(8), 3);
}

TEST(PathFinding, HookWalls)
{
    char wall[64] = {
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 1, 0, 0,
        0, 0, 0, 0, 0, 1, 0, 0,
        0, 0, 0, 0, 0, 1, 1, 1,
        1, 1, 1, 0, 0, 0, 0, 0,
        0, 0, 1, 0, 0, 0, 0, 0,
        0, 0, 1, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0
    };
    auto result = GDT::findPath<char, int>(
        wall,
        40,
        23,
        8,
        64,
        [] (const char* array, const int& pos) {
            return array[pos] == 1;
        });

    unsigned int length = 0;
    int pos = 40;
    while(pos != 23)
    {
        pos = result.at(pos);
        ++length;
        if(length > 100)
        {
            ASSERT_FALSE("Length is too long");
        }
    }
    EXPECT_EQ(length, 18);
}

TEST(PathFinding, TwoPossiblePaths)
{
    char wall[72] = {
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 1, 0, 0, 0,
        0, 0, 0, 0, 1, 0, 0, 0,
        0, 0, 0, 0, 1, 0, 0, 0,
        0, 0, 0, 0, 1, 0, 0, 0,
        0, 0, 0, 0, 1, 0, 0, 0,
        0, 0, 0, 0, 1, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0
    };

    auto result = GDT::findPath<char, int>(
        wall,
        32,
        39,
        8,
        72,
        [] (const char* array, const int& index) {
            return array[index] == 1;
        });

    unsigned int length = 0;
    int pos = 32;
    while(pos != 39)
    {
        pos = result.at(pos);
        ++length;
        if(length > 100)
        {
            ASSERT_FALSE("Length is too long");
        }
    }
    EXPECT_EQ(length, 13);
}
