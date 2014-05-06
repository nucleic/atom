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


#define atomdict_cast( o ) ( reinterpret_cast<AtomDict*>( o ) )
#define atomcdict_cast( o ) ( reinterpret_cast<AtomCDict*>( o ) )
#define AtomDict_Check( o ) ( PyObject_TypeCheck( o, &AtomDict_Type ) )
#define AtomCDict_Check( o ) ( PyObject_TypeCheck( o, &AtomCDict_Type ) )


typedef struct {
    PyDictObject dict;
    Member* key_validator;
    Member* value_validator;
    CAtomPointer* pointer;
} AtomDict;


typedef struct {
    AtomDict atomdict;
    Member* member;
} AtomCDict;


extern PyTypeObject AtomDict_Type;


extern PyTypeObject AtomCDict_Type;


PyObject*
AtomDict_New( CAtom* atom, Member* key_validator, Member* value_validator );


PyObject*
AtomCDict_New( CAtom* atom, Member* key_validator, Member* value_validator, Member* member );


int
import_atomdict();