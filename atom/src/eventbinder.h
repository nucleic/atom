/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2019, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file LICENSE, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once
#include "catom.h"
#include "member.h"

namespace atom
{


// POD struct - all member fields are considered private
struct EventBinder
{
	PyObject_HEAD
    Member* member;
    CAtom* atom;

	static PyType_Spec TypeObject_Spec;

    static PyTypeObject* TypeObject;

	static bool Ready();

    static PyObject* New( Member* member, CAtom* atom );

    static bool TypeCheck( PyObject* ob )
	{
		return PyObject_TypeCheck( ob, TypeObject ) != 0;
	}

};


}  // namespace atom
