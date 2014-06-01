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


struct CAtom;


int import_member();


extern PyTypeObject Member_Type;


struct Member
{
    PyObject_HEAD;
    PyObject* metadata;
    PyObject* default_value_context;
    PyObject* validate_context;
    PyObject* post_validate_context;
    PyObject* post_setattr_context;
    uint8_t default_value_mode;
    uint8_t validate_mode;
    uint8_t post_validate_mode;
    uint8_t post_setattr_mode;
};


inline int Member_Check( PyObject* op )
{
    return PyObject_TypeCheck( op, &Member_Type );
}


// newref on success, null and exception on failure
PyObject* Member_GetAttr( Member* member,
                          CAtom* atom,
                          PyStringObject* name,
                          PyObject** slot );


// 0 on sucess, -1 and exception on failure
int Member_SetAttr( Member* member,
                    CAtom* atom,
                    PyStringObject* name,
                    PyObject* value,
                    PyObject** slot );
