/*-----------------------------------------------------------------------------
| Copyright (c) 2013, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once
#include "pythonhelpers.h"
#include "catom.h"
#include "catompointer.h"
#include "member.h"


#define atomlist_cast( o ) ( reinterpret_cast<AtomList*>( o ) )
#define AtomList_Check( o ) ( PyObject_TypeCheck( o, &AtomList_Type ) )


typedef struct {
    PyListObject list;
    Member* validator;
    CAtomPointer* pointer;
} AtomList;


extern PyTypeObject AtomList_Type;


PyObject*
AtomList_New( Py_ssize_t size, CAtom* atom, Member* validator );


int
import_atomlist();
