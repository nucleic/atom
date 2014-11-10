/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic
|
| Distributed under the terms of the BSD 3-Clause License.
|
| The full license is in the file LICENSE, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once

#include <Python.h>


namespace atom
{

namespace utils
{

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
