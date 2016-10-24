
#ifndef GDT_ENDIANNESS_HPP
#define GDT_ENDIANNESS_HPP

#include <cstdint>

namespace GDT
{
    bool isBigEndian()
    {
        union {
            uint32_t i;
            char c[4];
        } bint = { 0x01020304 };

        return bint.c[0] == 1;
    }
}

#endif

