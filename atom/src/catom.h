/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once

#include <Python.h>
#include "inttypes.h"


struct ClassMap;


int
import_catom();


extern PyTypeObject CAtom_Type;


struct CAtom
{
    enum Flag
    {
    };

    // all fields are considered private
    PyObject_HEAD
    ClassMap* class_map;
    PyObject** slots;
    uint32_t flags;
};


inline int
CAtom_Check( PyObject* op )
{
    return PyObject_TypeCheck( op, &CAtom_Type );
}



inline bool
CAtom_TestFlag( CAtom* atom, CAtom::Flag flag )
{
    return ( atom->flags & static_cast<uint32_t>( flag ) ) != 0;
}


inline void
CAtom_SetFlag( CAtom* atom, CAtom::Flag flag, bool on=true )
{
    if( on )
    {
        atom->flags |= static_cast<uint32_t>( flag );
    }
    else
    {
        atom->flags &= ~( static_cast<uint32_t>( flag ) );
    }
}
