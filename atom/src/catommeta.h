/*-----------------------------------------------------------------------------
 | Copyright (c) 2025, Nucleic Development Team.                           *
 |
 | Distributed under the terms of the Modified BSD License.
 |
 | The full license is in the file LICENSE, distributed with this software.
 |----------------------------------------------------------------------------*/
#pragma once

#include <cppy/cppy.h>
#include "platstdint.h"
#define catommeta_cast( o ) ( reinterpret_cast<atom::CAtomMeta*>( o ) )


namespace atom
{

// Base type for all CAtom classes
// see cpythons's testapi/heaptype.c'
struct CAtomMeta
{
    PyHeapTypeObject base;
    PyObject* atom_members;
    uint16_t slot_count;

    static PyType_Spec TypeObject_Spec;
    static PyTypeObject* TypeObject;
    static bool Ready();
    static int TypeCheck( PyObject* object )
    {
        return PyObject_TypeCheck( object, TypeObject );
    }

    // All of these functions may return 0 and set an error
    // if the class was not properly initialized
    PyObject* init_subclass();
    PyObject* members(); // New reference.
    PyObject* get_member( PyObject* name );

};

}
