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


int
import_member();


extern PyTypeObject Member_Type;


struct Member
{
    PyObject_HEAD
    uint16_t slot_index;
};


inline int
Member_Check( PyObject* object )
{
    return PyObject_TypeCheck( object, &Member_Type );
}
