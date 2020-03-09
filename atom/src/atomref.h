/*-----------------------------------------------------------------------------
| Copyright (c) 2013, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file LICENSE, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once
#include "catom.h"
#include "catompointer.h"


namespace atom
{


// POD struct - all member fields are considered private
struct AtomRef
{
	PyObject_HEAD
    CAtomPointer pointer;  // constructed with placement new

	static PyType_Spec TypeObject_Spec;

    static PyTypeObject* TypeObject;

	static bool Ready();

    static bool TypeCheck( PyObject* ob )
	{
		return PyObject_TypeCheck( ob, TypeObject ) != 0;
	}

};


namespace SharedAtomRef
{

void
clear( CAtom* atom );

}  // namespace SharedAtomRef

}  // namespace atom
