/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once

#include "py23compat.h"

#include <Python.h>
#include <cppy/cppy.h>


namespace atom
{

inline Py_ssize_t sys_getsizeof( PyObject* pyo )
{
	PyObject* sysdict = PyThreadState_GET()->interp->sysdict;
	PyObject* getsizeof = PyDict_GetItemString( sysdict, "getsizeof" );
	if( !getsizeof )
	{
		cppy::system_error( "failed to load sys.getsizeof" );
		return -1;
	}
	cppy::ptr args( PyTuple_Pack( 1, pyo ) );
	if( !args )
	{
		return -1;
	}
	cppy::ptr size( PyObject_Call( getsizeof, args.get(), 0 ) );
	if( !size )
	{
		return -1;
	}
	return Py23Int_AsSsize_t( size.get() );
}

} // namespace atom
