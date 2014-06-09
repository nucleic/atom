/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once

#include <Python.h>


struct Member
{
    PyObject_HEAD;
    PyObject* default_handler;
    PyObject* validate_handler;
    PyObject* post_validate_handler;
    PyObject* post_setattr_handler;
};


extern PyTypeObject Member_Type;


inline int Member_Check( PyObject* op )
{
    return PyObject_TypeCheck( op, &Member_Type );
}


int import_member();
