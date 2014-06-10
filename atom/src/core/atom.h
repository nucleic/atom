/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once

#include <Python.h>
#include <utils/stdint.h>


struct ClassMap;


struct Atom
{
    enum Flag
    {};

    PyObject_HEAD;
    ClassMap* class_map;
    PyObject** slots;
    uint32_t flags;
};


inline bool Atom_TestFlag( Atom* atom, Atom::Flag flag )
{
    return ( atom->flags & static_cast<uint32_t>( flag ) ) != 0;
}


inline void Atom_SetFlag( Atom* atom, Atom::Flag flag, bool on = true )
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


extern PyTypeObject Atom_Type;


inline bool Atom_Check( PyObject* ob )
{
    return PyObject_TypeCheck( ob, &Atom_Type ) != 0;
}


int import_atom();
