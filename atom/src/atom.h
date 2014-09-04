/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once

#include "callback_set.h"
#include "signal.h"

#include <cppy/cppy.h>
#include <Python.h>
#include <utility>
#include <vector>


namespace atom
{

// POD struct - all member fields are considered private
struct Atom
{
	typedef std::vector<cppy::ptr> ValueVector;
	typedef std::pair<cppy::ptr, CallbackSet> CSPair;
	typedef std::vector<CSPair> CSVector;

	PyObject_HEAD
	PyObject* m_weaklist;
	PyObject* m_members;
	CSVector* m_cbsets;
	ValueVector m_values;  // instantiated with placement new

	static PyTypeObject TypeObject;

	static bool Ready();

	static bool TypeCheck( PyObject* ob )
	{
		return PyObject_TypeCheck( ob, &TypeObject ) != 0;
	}

	static bool RegisterMembers( PyTypeObject* type, PyObject* members );

	static PyObject* LookupMembers( PyTypeObject* type );

	void connect( Signal* sig, PyObject* callback );

	void disconnect();

	void disconnect( Signal* sig );

	void disconnect( Signal* sig, PyObject* callback );

	void emit( Signal* sig, PyObject* args, PyObject* kwargs = 0 );
};

}  // namespace atom
