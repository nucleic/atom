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

struct Atom;


// POD struct - all member fields are considered private
struct Signal
{
	PyObject_HEAD

	static PyTypeObject TypeObject;

	static bool Ready();

	static bool TypeCheck( PyObject* ob )
	{
		return PyObject_TypeCheck( ob, &TypeObject ) != 0;
	}
};


// POD struct - all member fields are considered private
struct BoundSignal
{
	PyObject_HEAD
	Signal* m_signal;
	Atom* m_atom;

	static PyTypeObject TypeObject;

	static bool Ready();

	static bool TypeCheck( PyObject* ob )
	{
		return PyObject_TypeCheck( ob, &TypeObject ) != 0;
	}

	static PyObject* Create( Signal* sig, Atom* atom );
};

} // namespace atom
