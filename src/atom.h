/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once

#include "callbackset.h"
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
	typedef std::pair<cppy::ptr, CallbackSet> CSPair;
	typedef std::vector<CSPair> CSVector;

	PyObject_VAR_HEAD
	PyObject* m_weaklist;
	CSVector* m_cbsets;
	PyObject* m_members;    // members are shared by all instances
	PyObject* m_values[1];  // values are inlined in the struct

	static PyTypeObject TypeObject;

	static bool Ready();

	static bool TypeCheck( PyObject* ob )
	{
		return PyObject_TypeCheck( ob, &TypeObject ) != 0;
	}

	static bool RegisterMembers( PyTypeObject* type, PyObject* members );

	static PyObject* LookupMembers( PyTypeObject* type );

	static PyObject* Sender();

	static PyObject* Connect( Atom* atom, Signal* sig, PyObject* callback );

	static void Disconnect( Atom* atom );

	static void Disconnect( Atom* atom, Signal* sig );

	static void Disconnect( Atom* atom, Signal* sig, PyObject* callback );

	static void Emit( Atom* atom, Signal* sig, PyObject* args, PyObject* kwargs = 0 );
};

} // namespace atom
