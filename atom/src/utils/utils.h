/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once

#include <Python.h>
#include "stdint.h"


namespace utils
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


inline size_t pystr_hash( PyStringObject* op )
{
    long hash = op->ob_shash;
    if( hash == -1 )
    {
        hash = PyObject_Hash( ( PyObject* )op );
    }
    return static_cast<size_t>( hash );
}


inline bool pystr_equal( PyStringObject* a, PyStringObject* b )
{
    if( a == b )
    {
        return true;
    }
    if( Py_SIZE( a ) == Py_SIZE( b ) )
    {
        return memcmp( a->ob_sval, b->ob_sval, Py_SIZE( a ) ) == 0;
    }
    return false;
}


} // namespace utils
