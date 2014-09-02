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

inline void printAndClearError()
{
	if( PyErr_Occurred() )
	{
		PyErr_Print();
	}
}


inline bool safeCmpEqual( PyObject* a, PyObject* b )
{
	int ok = PyObject_RichCompareBool( a, b, Py_EQ );
	if( ok == 1 )
	{
		return true;
	}
	if( ok == 0 )
	{
		return false;
	}
	printAndClearError();
	return false;
}


inline bool safeCmpLess( PyObject* a, PyObject* b )
{
	int ok = PyObject_RichCompareBool( a, b, Py_LT );
	if( ok == 1 )
	{
		return true;
	}
	if( ok == 0 )
	{
		return false;
	}
	printAndClearError();
	return false;
}


inline bool safeIsTruthy( PyObject* pyo )
{
	int ok = PyObject_IsTrue( pyo );
	if( ok == 1 )
	{
		return true;
	}
	if( ok == 0 )
	{
		return false;
	}
	printAndClearError();
	return false;
}


inline bool safeSetContains( PyObject* set, PyObject* value )
{
	int ok = PySet_Contains( set, value );
	if( ok == 1 )
	{
		return true;
	}
	if( ok == 0 )
	{
		return false;
	}
	printAndClearError();
	return false;
}


inline void safeSetAdd( PyObject* set, PyObject* value )
{
	if( PySet_Add( set, value ) < 0 )
	{
		printAndClearError();
	}
}


inline void safeSetDiscard( PyObject* set, PyObject* value )
{
	if( PySet_Discard( set, value ) < 0 )
	{
		printAndClearError();
	}
}


inline void safeCallObject( PyObject* cb, PyObject* args, PyObject* kwargs )
{
	cppy::ptr ignored( PyObject_Call( cb, args, kwargs ) );
	if( !ignored )
	{
		printAndClearError();
	}
}

} // namespace atom
