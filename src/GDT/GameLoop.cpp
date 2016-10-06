
#include "GameLoop.hpp"

#include <chrono>
#include <thread>

void GDT::IntervalBasedGameLoop(const bool* runFlag,
                                const std::function<void(float)>& updateFunction,
                                const std::function<void()>& drawFunction,
                                const unsigned int& fpsLimit,
                                const float& intervalLength)
{
    auto time = std::chrono::steady_clock::now();
    decltype(time) ntime;
    std::chrono::duration<float> duration;
    std::chrono::duration<float> lDuration;
    if(fpsLimit > 0)
    {
        lDuration = std::chrono::duration<float>(2.0f / (float)fpsLimit);
    }
    float ticks = 0;
    while(*runFlag)
    {
        // get elapsed time
        ntime = std::chrono::steady_clock::now();
        duration = ntime - time;
        ticks += duration.count();
        // update loop
        while(ticks >= intervalLength)
        {
            ticks -= intervalLength;
            updateFunction(intervalLength);
        }
        // draw
        drawFunction();
        // sleep
        if(duration < lDuration)
        {
            std::this_thread::sleep_for(lDuration - duration);
        }
        time = ntime;
    }
}

