/*-----------------------------------------------------------------------------
| Copyright (c) 2019, Nucleic
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


#define atomdict_cast( o ) ( reinterpret_cast<AtomDict*>( o ) )


typedef struct
{
	PyDictObject dict;
	Member* m_key_validator;
	Member* m_value_validator;
    CAtomPointer* pointer;

} AtomDict;


extern PyTypeObject AtomDict_Type;


PyObject*
AtomDict_New( CAtom* atom, Member* key_validator, Member* value_validator );


int
AtomDict_Update( AtomDict* dict, PyObject* value );


int
import_atomdict();
