/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2019, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file LICENSE, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once
#include "catompointer.h"


namespace atom
{

// POD struct - all member fields are considered private
struct MethodWrapper
{
	PyObject_HEAD
    PyObject* im_func;
    PyObject* im_selfref;

	static PyType_Spec TypeObject_Spec;

    static PyTypeObject* TypeObject;

    static bool Ready();

    static PyObject* New( PyObject* method );

    static bool TypeCheck( PyObject* ob )
	{
		return PyObject_TypeCheck( ob, TypeObject ) != 0;
	}

};


// POD struct - all member fields are considered private
struct AtomMethodWrapper
{
	PyObject_HEAD
    PyObject* im_func;
    CAtomPointer pointer;  // constructed with placement new

	static PyType_Spec TypeObject_Spec;

    static PyTypeObject* TypeObject;

    static bool Ready();

    static bool TypeCheck( PyObject* ob )
	{
		return PyObject_TypeCheck( ob, TypeObject ) != 0;
	}

};

}  // namespace atom
