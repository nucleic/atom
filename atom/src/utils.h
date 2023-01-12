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
    if( Py_TYPE( first ) == Py_TYPE( second ) ) {
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
    const char* fn = PyNumber_Check( first ) ? "" : Py_TYPE( first )->tp_name;
    const char* sn = PyNumber_Check( second ) ? "" : Py_TYPE( second )->tp_name;
    int c = strcmp( fn, sn );
    if( c < 0 )
        return -1;
    if( c > 0 )
        return 1;

    // Finally, fall back to comparing type pointers.
    Py_uintptr_t ftp = reinterpret_cast<Py_uintptr_t>( Py_TYPE( first ) );
    Py_uintptr_t stp = reinterpret_cast<Py_uintptr_t>( Py_TYPE( second ) );
    return ftp < stp ? -1 : 1;
}


/**
 * A safe richcomparison that will never fail and fallback to 3way compare
 * if the conventional rich compare fails.
 */
inline bool safe_richcompare( PyObject* first, PyObject* second, int opid )
{
    // Start with Python's rich compare.
    int r = PyObject_RichCompareBool( first, second, opid );

    // Handle a successful comparison.
    if( r == 1 )
        return true;
    if( r == 0 )
        return false;

    // Clear the error if one happened because we attempted an invalid
    // comparison.
    if( PyErr_Occurred() )
        PyErr_Clear();

    // Fallback to the Python 2 default 3 way compare.
    int c = fallback_3way_compare( first, second );

    // Convert the 3way comparison result based on the `opid`.
    switch (opid) {
    case Py_EQ: return c == 0;
    case Py_NE: return c != 0;
    case Py_LE: return c <= 0;
    case Py_GE: return c >= 0;
    case Py_LT: return c < 0;
    case Py_GT: return c > 0;
    }

    // Return `false` if the `opid` is not handled.
    return false;
}

inline bool safe_richcompare( cppy::ptr first, PyObject* second, int opid )
{
    return safe_richcompare( first.get(), second, opid );
}


inline bool safe_richcompare( PyObject* first, cppy::ptr second, int opid )
{
    return safe_richcompare( first, second.get(), opid );
}


inline bool safe_richcompare( cppy::ptr first, cppy::ptr second, int opid )
{
    return safe_richcompare( first.get(), second.get(), opid );
}


} // namespace utils

} // namespace atom
