#ifndef GDT_PATH_FINDING_HPP
#define GDT_PATH_FINDING_HPP

#include <type_traits>
#include <unordered_map>
#include <queue>
#include <functional>
#include <cmath>

#include <GDT/Internal/PathFindingInternal.hpp>

namespace GDT
{
    /*!
     * \brief Returns an unordered_map that has the shortest path from start to end.
     *
     * Starting from index "start", the unordered_map's value at key "start" is
     * the next index toward "end". The value associated with key "end" is also
     * the index "end".
     *
     * Note that the array parameter is a 1-dimensional array but it will be
     * used as a 2-dimensional array, which is why the "width" parameter is
     * required. X and y are calculated by the following:
     *
     * \code{.cpp}
     * IndexType x = index % width;
     * IndexType y = index / width;
     * \endcode
     */
    template <typename T, typename IndexType,
        typename std::enable_if<std::is_integral<IndexType>::value>::type* = 0>
    std::unordered_map<IndexType, IndexType> findPath(
        const T *array,
        const IndexType& start,
        const IndexType& end,
        const IndexType& width,
        const IndexType& size,
        const std::function<bool(const T*, const IndexType&)>& isObstacle
    );
}

#include "PathFinding.inl"

#endif
