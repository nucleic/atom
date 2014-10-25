/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic
|
| Distributed under the terms of the BSD 3-Clause License.
|
| The full license is in the file LICENSE, distributed with this software.
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

	PyObject* single()  // for GC traversing only
	{
		return m_single.get();
	}

	PyObject* extras()  // for GC traversing only
	{
		return m_extras.get();
	}

private:

	cppy::ptr m_single;
	cppy::ptr m_extras;
};

} // namespace atom
