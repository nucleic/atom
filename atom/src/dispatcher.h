/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once

#include "callback_set.h"

#include <Python.h>
#include <cppy/cppy.h>
#include <utility>
#include <vector>


namespace atom
{

class Dispatcher
{
public:

	Dispatcher() { }

	~Dispatcher() { }

	void connect( PyObject* name, PyObject* callback );

	void disconnect();

	void disconnect( PyObject* name );

	void disconnect( PyObject* name, PyObject* callback );

	void emit( PyObject* name, PyObject* args, PyObject* kwargs = 0 );

	static bool Ready();

	static PyTypeObject TypeObject;

private:

	typedef std::pair<cppy::ptr, CallbackSet> CBSPair;
	typedef std::vector<CBSPair> CBSVector;

	Dispatcher( Dispatcher& );
	Dispatcher& operator=( Dispatcher& );

	CBSVector::iterator find( PyObject* name );

	CBSVector::iterator lowerBound( PyObject* name );

	CBSVector m_cbsets;
};

} // namespace atom
