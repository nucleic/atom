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


struct Descriptor;


int
import_catom();


extern PyTypeObject CAtom_Type;


struct CAtom
{
    typedef uint16_t flags_t;

    enum Flag
    {
        NotificationsEnabled = 0x1,
    };

    PyObject_HEAD
    Descriptor* descriptor;
    PyObject** slots;
    uint16_t allocated;
    flags_t flags;
};


inline int
CAtom_Check( PyObject* object )
{
    return PyObject_TypeCheck( object, &CAtom_Type );
}
