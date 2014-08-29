/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once

#include <cppy/cppy.h>
#include <Python.h>
#include <vector>


namespace atom
{

// POD struct - all member fields are considered private
struct Atom
{
	typedef std::vector<cppy::ptr> ValueVector;

	PyObject_HEAD
	PyObject* m_class_members;
	PyObject* m_extra_members;
	ValueVector m_values;  // instantiated with placement new

	static PyTypeObject TypeObject;

	static bool Ready();

	static bool TypeCheck( PyObject* ob )
	{
		return PyObject_TypeCheck( ob, &TypeObject ) != 0;
	}

	static PyObject* RegisterMembers( PyTypeObject* type, PyObject* members );

	static PyObject* LookupMembers( PyTypeObject* type );
};

}  // namespace atom
