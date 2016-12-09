
#ifndef GDT_COLLISION_DETECTION_HPP
#define GDT_COLLISION_DETECTION_HPP

namespace GDT
{
    /// Returns true if the point (x,y) is within a convex polygon.
    /**
        The shape of the polygon defined by arrayOfPairs must be convex.
        Otherwise, behavior is undefined.
        arrayOfPairs must be an array of at least size 3 of an array size 2.
        size must be the number of size-2-arrays within arrayOfPairs.
        Thus if arrayOfPairs is defined as \code{.cpp} float arrayOfPairs[4][2]
        \endcode , then size must be 4.
    */
    bool isWithinPolygon(float (*arrayOfPairs)[2], unsigned int size, float x, float y);

    /// Returns true if the point (x,y) is within a convex polygon.
    /**
        The shape of the polygon defined by arrayOfPairs must be convex.
        Otherwise, behavior is undefined.
        arrayOfPairs must be an array of at least size 6 and even.
        arraySize must be the size of arrayOfPairs.
        Thus if arrayOfPairs is defined as float arrayOfPairs[8], then
        arraySize must be 8.
        If the size of arrayOfPairs is not even, then behavior is undefined.
    */
    bool isWithinPolygon(float* arrayOfPairs, unsigned int arraySize, float x, float y);
}

#endif

