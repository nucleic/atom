/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic
|
| Distributed under the terms of the BSD 3-Clause License.
|
| The full license is in the file LICENSE, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once

#include "py23compat.h"

#include <Python.h>
#include <cppy/cppy.h>


namespace atom
{

namespace utils
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


inline bool is_type_or_tuple_of_types( PyObject* pyo )
{
	if( PyType_Check( pyo ) )
	{
		return true;
	}
	if( !PyTuple_Check( pyo ) )
	{
		return false;
	}
	Py_ssize_t count = PyTuple_GET_SIZE( pyo );
	for( Py_ssize_t i = 0; i < count; ++i )
	{
		if( !is_type_or_tuple_of_types( PyTuple_GET_ITEM( pyo, i ) ) )
		{
			return false;
		}
	}
	return true;
}

} // namespace utils

} // namespace atom
