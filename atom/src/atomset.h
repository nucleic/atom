/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic
|
| Distributed under the terms of the BSD 3-Clause License.
|
| The full license is in the file LICENSE, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once
#include <cppy/cppy.h>
#include "catom.h"
#include "catompointer.h"
#include "member.h"


#define atomset_cast( o ) ( reinterpret_cast<AtomSet*>( o ) )


typedef struct
{
	PySetObject set;
	Member* m_value_validator;
    CAtomPointer* pointer;

} AtomSet;


extern PyTypeObject AtomSet_Type;


PyObject*
AtomSet_New( CAtom* atom, Member* validator );


int
AtomSet_Update( AtomSet* set, PyObject* other );


int
import_atomset();
