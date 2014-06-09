/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once

#include <Python.h>


struct Atom;


enum AtomFlag
{};


extern PyTypeObject Atom_Type;


inline bool Atom_Check( PyObject* ob )
{
    return PyObject_TypeCheck( ob, &Atom_Type ) != 0;
}


int import_atom();
