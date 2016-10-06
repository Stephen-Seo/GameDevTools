
#include "gtest/gtest.h"

#include <iostream>

#include <GDT/GameLoop.hpp>

TEST(GameLoop, FpsLimit)
{
    bool runflag = true;
    unsigned int counter = 0;
    const float limit = 5.5f;
    float ltimer = 0;
    float ctimer = 0;
    unsigned int fpsAmounts[5];
    unsigned char fpsAmountsIndex = 0;
    auto update = [&runflag, &counter, &ltimer, &ctimer, &limit, &fpsAmounts, &fpsAmountsIndex] (float deltaTime) {
        ltimer += deltaTime;
        ctimer += deltaTime;

        if(ltimer >= limit)
        {
            runflag = false;
        }

        if(ctimer >= 1.0f)
        {
            std::cout << "Counter at 1s is " << counter << std::endl;
            ctimer = 0;
            fpsAmounts[fpsAmountsIndex] = counter;
            fpsAmountsIndex = (fpsAmountsIndex + 1) % 5;
            counter = 0;
        }

        float test[255];
        for(unsigned int i = 0; i < 255; ++i)
        {
            test[i] = i * 10.0f;
        }
    };
    auto draw = [&counter] () {
        ++counter;
    };

    GDT::IntervalBasedGameLoop(&runflag, update, draw, 60);

    float avgFps = 0;
    for(unsigned int i = 0; i < 5; ++i)
    {
        avgFps += fpsAmounts[i];
    }
    avgFps /= 5.0f;

    EXPECT_GT(avgFps, 55.0f);
    EXPECT_LT(avgFps, 65.0f);

    runflag = true;
    ltimer = 0.0f;
    ctimer = 0.0f;
    counter = 0;
    GDT::IntervalBasedGameLoop(&runflag, update, draw, 45);

    avgFps = 0;
    for(unsigned int i = 0; i < 5; ++i)
    {
        avgFps += fpsAmounts[i];
    }
    avgFps /= 5.0f;

    EXPECT_GT(avgFps, 40.0f);
    EXPECT_LT(avgFps, 50.0f);
}

