/*-----------------------------------------------------------------------------
| Copyright (c) 2014-2019, Nucleic
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


#define atomset_cast( o ) ( reinterpret_cast<atom::AtomSet*>( o ) )


namespace atom
{

// POD struct - all member fields are considered private
struct AtomSet
{
    PySetObject set;
	Member* m_value_validator;
    CAtomPointer* pointer;

	static PyType_Spec TypeObject_Spec;

    static PyTypeObject* TypeObject;

	static bool Ready();

    static PyObject* New( CAtom* atom, Member* validator );

    static int Update( AtomSet* set, PyObject* value );

    static bool TypeCheck( PyObject* ob )
	{
		return PyObject_TypeCheck( ob, TypeObject ) != 0;
	}

};

} // namespace atom
