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
struct Atom
{
	PyObject_VAR_HEAD
	PyObject* m_weaklist;
	PyObject* m_callbacks;
	PyObject* m_values[1];  // values are inlined in the struct

	static PyTypeObject TypeObject;

	static bool Ready();

	static bool TypeCheck( PyObject* ob )
	{
		return PyObject_TypeCheck( ob, &TypeObject ) != 0;
	}
};

} // namespace atom
