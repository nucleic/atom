/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once

#include "stdint.h"


namespace atom
{

inline uint32_t next_power_of_2( uint32_t n ) // n must be greater than zero
{
    n = n - 1;
    n = n | ( n >> 1 );
    n = n | ( n >> 2 );
    n = n | ( n >> 4 );
    n = n | ( n >> 8 );
    n = n | ( n >> 16 );
    return n + 1;
}

} // namespace atom
