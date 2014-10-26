/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic
|
| Distributed under the terms of the BSD 3-Clause License.
|
| The full license is in the file LICENSE, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once

#include <Python.h>


namespace atom
{

// POD struct - all member fields are considered private
struct MemberChange
{
	PyObject_HEAD
	PyObject* m_object;
	PyObject* m_name;
	PyObject* m_old_value;
	PyObject* m_new_value;

	static PyTypeObject TypeObject;

	static bool Ready();

	static bool TypeCheck( PyObject* ob )
	{
		return PyObject_TypeCheck( ob, &TypeObject ) != 0;
	}

	static PyObject* Create( PyObject* object, PyObject* name, PyObject* old_value, PyObject* new_value );
};

} // namespace atom
