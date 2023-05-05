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


#define atomdict_cast( o ) ( reinterpret_cast<atom::AtomDict*>( o ) )
#define defaultatomdict_cast( o ) ( reinterpret_cast<atom::DefaultAtomDict*>( o ) )


namespace atom
{

// POD struct - all member fields are considered private
struct AtomDict
{
	PyDictObject dict;
	Member* m_key_validator;
	Member* m_value_validator;
    CAtomPointer* pointer;

	static PyType_Spec TypeObject_Spec;

    static PyTypeObject* TypeObject;

	static bool Ready();

    static PyObject* New( CAtom* atom, Member* key_validator, Member* value_validator );

    static int Update( AtomDict* dict, PyObject* value );

    static bool TypeCheck( PyObject* ob )
	{
		return PyObject_TypeCheck( ob, TypeObject ) != 0;
	}

};

// POD struct - all member fields are considered private
struct DefaultAtomDict
{
	AtomDict dict;
	PyObject* factory;

	static PyType_Spec TypeObject_Spec;

    static PyTypeObject* TypeObject;

	static bool Ready();

    static PyObject* New(
		CAtom* atom, Member* key_validator, Member* value_validator, PyObject* factory
	);

    static bool TypeCheck( PyObject* ob )
	{
		return PyObject_TypeCheck( ob, TypeObject ) != 0;
	}

};

} // namespace atom
