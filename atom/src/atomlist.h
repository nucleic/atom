/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2019, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file LICENSE, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once
#include <cppy/cppy.h>
#include "catom.h"
#include "catompointer.h"
#include "member.h"


#define atomlist_cast( o ) ( reinterpret_cast<atom::AtomList*>( o ) )
#define atomclist_cast( o ) ( reinterpret_cast<atom::AtomCList*>( o ) )

namespace atom
{


// POD struct - all member fields are considered private
struct AtomList
{
	PyListObject list;
    Member* validator;
    CAtomPointer* pointer;

	static PyType_Spec TypeObject_Spec;

    static PyTypeObject* TypeObject;

	static bool Ready();

    static PyObject* New( Py_ssize_t size, CAtom* atom, Member* validator );

    static bool TypeCheck( PyObject* ob )
	{
		return PyObject_TypeCheck( ob, TypeObject ) != 0;
	}

};

bool
init_containerlistchange();


// POD struct - all member fields are considered private
struct AtomCList
{
	PyListObject list;
    Member* validator;
    CAtomPointer* pointer;
    Member* member;

	static PyType_Spec TypeObject_Spec;

    static PyTypeObject* TypeObject;

	static bool Ready();

    static PyObject* New( Py_ssize_t size, CAtom* atom, Member* validator, Member* member );

    static bool TypeCheck( PyObject* ob )
	{
		return PyObject_TypeCheck( ob, TypeObject ) != 0;
	}

};


}
