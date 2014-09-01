/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once

#include <Python.h>
#include <cppy/cppy.h>
#include <utility>
#include <vector>


namespace atom
{

class CallbackSet
{

public:

	CallbackSet() { }

	CallbackSet( PyObject* callback ) :
		m_single( callback, true ) { }

	~CallbackSet() { }

	void add( PyObject* callback );

	void remove( PyObject* callback );

	void dispatch( PyObject* args, PyObject* kwargs = 0 );

private:

	cppy::ptr m_single;
	cppy::ptr m_set;
};


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

private:

	typedef std::pair<cppy::ptr, CallbackSet> CBSPair;
	typedef std::vector<CBSPair> CBSVector;

	Dispatcher( Dispatcher& );
	Dispatcher& operator=( Dispatcher& );

	CBSVector::iterator find( PyObject* name );

	CBSVector m_cbsets;
};

} // namespace atom
