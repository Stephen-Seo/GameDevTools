
#ifndef GDT_GAME_LOOP_HPP
#define GDT_GAME_LOOP_HPP

#include <functional>

namespace GDT
{
    void IntervalBasedGameLoop(const bool* runFlag,
                               const std::function<void(float)>& updateFunction,
                               const std::function<void()>& drawFunction,
                               const unsigned int& fpsLimit = 60,
                               const float& intervalLength = 1.0f / 90.0f);
}

#endif

