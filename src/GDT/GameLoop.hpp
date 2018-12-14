
#ifndef GDT_GAME_LOOP_HPP
#define GDT_GAME_LOOP_HPP

#include <functional>

namespace GDT
{
    /*!
     * \brief Run a game loop with provided functions and interval.
     *
     * The update function will be called multiple times with the interval as
     * its parameter, but the draw function will only be called at the specified
     * fpsLimit.
     *
     * If runFlag is set to false, the game loop will end.
     *
     * The parameter given to the update function should be considered the
     * "delta time" between invocations of draws. The interval should be less
     * than the actual frame rate interval, so update will be called multiple
     * times until the sum of "delta times" of invocations of update between
     * draws is equivalent to the time between draws.
     */
    void IntervalBasedGameLoop(const bool* runFlag,
                               const std::function<void(float)>& updateFunction,
                               const std::function<void()>& drawFunction,
                               const unsigned int& fpsLimit = 60,
                               const float& intervalLength = 1.0f / 120.0f);
}

#endif

