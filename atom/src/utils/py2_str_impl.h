/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once

#include <Python.h>


#define Py23StrObject PyStringObject
#define Py23Str_Check PyString_Check
#define Py23Str_FromString PyString_FromString


namespace atom
{

inline size_t py23_str_hash( Py23StrObject* op )
{
    long hash = op->ob_shash;
    if( hash == -1 )
    {
        hash = PyObject_Hash( reinterpret_cast<PyObject*>( op ) );
    }
    return static_cast<size_t>( hash );
}


inline bool py23_str_equal( Py23StrObject* a, Py23StrObject* b )
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

} // namespace atom
