
#include "CollisionDetection.hpp"

bool GDT::isWithinPolygon(float (*arrayOfPairs)[2], unsigned int size, float x, float y)
{
    float v0_x;
    float v0_y;
    float v1_x;
    float v1_y;
    unsigned int j;
    bool resultPositive;
    for(unsigned int i = 0; i < size; ++i)
    {
        // i is current index, j is next index
        j = (i + 1) % size;

        // get vertex 0, from arrayOfPairs[i] to arrayOfPairs[j]
        v0_x = arrayOfPairs[j][0] - arrayOfPairs[i][0];
        v0_y = arrayOfPairs[j][1] - arrayOfPairs[i][1];

        // get vertex 1, from arrayOfPairs[i] to given point (x,y)
        v1_x = x - arrayOfPairs[i][0];
        v1_y = y - arrayOfPairs[i][1];

        if(i == 0)
        {
            // first case, get positivity of cross product
            resultPositive = v0_x * v1_y - v1_x * v0_y > 0.0f;
        }
        else if(resultPositive != (v0_x * v1_y - v1_x * v0_y > 0.0f))
        {
            // next cross product doesn't match resultPositive
            return false;
        }
    }

    // all cross products reveal that point is always on one side of each line
    // of the polygon, thus is within polygon
    return true;
}

bool GDT::isWithinPolygon(float* arrayOfPairs, unsigned int arraySize, float x, float y)
{
    float v0_x;
    float v0_y;
    float v1_x;
    float v1_y;
    unsigned int j;
    bool resultPositive;
    for(unsigned int i = 0; i < arraySize; i += 2)
    {
        // i is current position x, j is next position x
        j = (i + 2) % arraySize;

        // get vertex 0, from arrayOfPairs[i] to arrayOfPairs[j]
        v0_x = arrayOfPairs[j] - arrayOfPairs[i];
        v0_y = arrayOfPairs[j+1] - arrayOfPairs[i+1];

        // get vertex 1, from arrayOfPairs[i] to given point (x,y)
        v1_x = x - arrayOfPairs[i];
        v1_y = y - arrayOfPairs[i+1];

        if(i == 0)
        {
            // first case, get positivity of cross product
            resultPositive = v0_x * v1_y - v1_x * v0_y > 0.0f;
        }
        else if(resultPositive != (v0_x * v1_y - v1_x * v0_y > 0.0f))
        {
            // next cross product doesn't match resultPositive
            return false;
        }
    }

    // all cross products reveal that point is always on one side of each line
    // of the polygon, thus is within polygon
    return true;
}

