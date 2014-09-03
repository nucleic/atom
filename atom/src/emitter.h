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

#include <Python.h>
#include <cppy/cppy.h>
#include <utility>
#include <vector>


namespace atom
{

// POD struct - all member fields are considered private
struct Emitter
{
	typedef std::pair<cppy::ptr, CallbackSet> CSPair;
	typedef std::vector<CSPair> CSVector;

	PyObject_HEAD
	CSVector* m_cbsets;

	static PyTypeObject TypeObject;

	static bool Ready();

	void connect( Signal* sig, PyObject* callback );

	void disconnect();

	void disconnect( Signal* sig );

	void disconnect( Signal* sig, PyObject* callback );

	void emit( Signal* sig, PyObject* args, PyObject* kwargs = 0 );
};

} // namespace atom
