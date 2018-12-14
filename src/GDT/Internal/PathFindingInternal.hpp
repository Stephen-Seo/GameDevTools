#ifndef GDT_PATH_FINDING_INTERNAL_HPP
#define GDT_PATH_FINDING_INTERNAL_HPP

namespace GDT
{
    namespace Internal
    {
        template <typename IndexType,
            typename std::enable_if<std::is_integral<IndexType>::value>::type* = 0>
        IndexType heuristic(IndexType width, IndexType a, IndexType b);

        template <typename IndexType, typename Enable = void>
        struct FindPathData
        {};

        template <typename IndexType>
        struct FindPathData
            <IndexType,
             typename std::enable_if<std::is_integral<IndexType>::value>::type>
        {
            FindPathData();
            FindPathData(IndexType i, IndexType priority);

            // copy
            FindPathData(const FindPathData& other) = default;
            FindPathData& operator = (const FindPathData& other) = default;

            // move
            FindPathData(FindPathData&& other) = default;
            FindPathData& operator = (FindPathData&& other) = default;

            IndexType index;
            IndexType priority;

            // is implemented in reverse: other.priority < priority
            bool operator < (const FindPathData<IndexType>& other) const;
        };
    }
}

#include "PathFindingInternal.inl"

#endif
