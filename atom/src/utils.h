/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2019, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file LICENSE, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once
#include <Python.h>
#include <cppy/cppy.h>


namespace atom
{

namespace utils
{

inline PyObject*
py_bool( bool val )
{
    return cppy::incref( val ? Py_True : Py_False );
}

inline bool
str_check( PyObject* obj )
{
    return (
        PyUnicode_CheckExact( obj ) ||
        PyObject_TypeCheck( obj, &PyUnicode_Type )
    );
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

/**
 * A fallback 3way comparison function for when PyObject_RichCompareBool
 * fails to compare "unorderable types" on Python 3.
 *
 * This is based on Python 2's `default_3way_compare`.
 *
 * This function will not change the Python exception state.
 */
inline int
fallback_3way_compare( PyObject* first, PyObject* second ) {
    // Compare pointer values if the types are the same.
    if( first->ob_type == second->ob_type ) {
        Py_uintptr_t fp = reinterpret_cast<Py_uintptr_t>( first );
        Py_uintptr_t sp = reinterpret_cast<Py_uintptr_t>( second );
        return (fp < sp) ? -1 : (fp > sp) ? 1 : 0;
    }

    // None is smaller than anything.
    if( first == Py_None )
        return -1;
    if( second == Py_None )
        return 1;

    // Compare based on type names, numbers are smaller.
    const char* fn = PyNumber_Check( first ) ? "" : first->ob_type->tp_name;
    const char* sn = PyNumber_Check( second ) ? "" : second->ob_type->tp_name;
    int c = strcmp( fn, sn );
    if( c < 0 )
        return -1;
    if( c > 0 )
        return 1;

    // Finally, fall back to comparing type pointers.
    Py_uintptr_t ftp = reinterpret_cast<Py_uintptr_t>( first->ob_type );
    Py_uintptr_t stp = reinterpret_cast<Py_uintptr_t>( second->ob_type );
    return ftp < stp ? -1 : 1;
}


} // namespace utils

} // namespace atom
