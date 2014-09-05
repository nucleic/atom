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


namespace atom
{

class CallbackSet
{
public:

	CallbackSet() { }

	CallbackSet( PyObject* callback ) : m_single( callback, true ) { }

	~CallbackSet() { }

	void add( PyObject* callback );

	void remove( PyObject* callback );

	void dispatch( PyObject* args, PyObject* kwargs = 0 );

	int traverse( visitproc visit, void* arg );

private:

	cppy::ptr m_single;
	cppy::ptr m_extras;
};

} // namespace atom
