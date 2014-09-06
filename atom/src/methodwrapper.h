/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once

#include <Python.h>


namespace atom
{

// POD struct - all member fields are considered private
struct MethodWrapper
{
	PyObject_HEAD
	PyObject* m_func;
	PyObject* m_obref;

	static PyTypeObject TypeObject;

	static bool Ready();

	static bool TypeCheck( PyObject* ob )
	{
		return Py_TYPE( ob ) == &TypeObject;
	}

	static PyObject* Create( PyMethodObject* method );
};

} // namespace atom
