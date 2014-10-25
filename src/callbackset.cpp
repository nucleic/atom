/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic
|
| Distributed under the terms of the BSD 3-Clause License.
|
| The full license is in the file LICENSE, distributed with this software.
|----------------------------------------------------------------------------*/
#include "callbackset.h"


namespace atom
{

namespace
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


inline bool safeListContains( PyObject* list, PyObject* value )
{
	for( Py_ssize_t i = 0; i < PyList_GET_SIZE( list ); ++i )
	{
		if( safeCmpEqual( PyList_GET_ITEM( list, i ), value ) )
		{
			return true;
		}
	}
	return false;
}


inline void safeListAdd( PyObject* list, PyObject* value )
{
	for( Py_ssize_t i = 0; i < PyList_GET_SIZE( list ); ++i )
	{
		if( safeCmpEqual( PyList_GET_ITEM( list, i ), value ) )
		{
			return;
		}
	}
	if( PyList_Append( list, value ) < 0 )
	{
		printAndClearError();
		return;
	}
}


inline void safeListDiscard( PyObject* list, PyObject* value )
{
	for( Py_ssize_t i = 0; i < PyList_GET_SIZE( list ); ++i )
	{
		if( safeCmpEqual( PyList_GET_ITEM( list, i ), value ) )
		{
			if( PyList_SetSlice( list, i, i + 1, 0 ) < 0 )
			{
				printAndClearError();
			}
			return;
		}
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


inline PyObject* copyList( PyObject* list )
{
	return PyList_GetSlice( list, 0, PyList_GET_SIZE( list ) );
}

} // namespace


void CallbackSet::add( PyObject* callback )
{
	if( m_single && safeCmpEqual( m_single.get(), callback ) )
	{
		return;
	}
	if( m_extras && safeListContains( m_extras.get(), callback ) )
	{
		return;
	}
	if( !m_single )
	{
		m_single = cppy::incref( callback );
		return;
	}
	if( !m_extras && !( m_extras = PyList_New( 0 ) ) )
	{
		printAndClearError();
		return;
	}
	safeListAdd( m_extras.get(), callback );
}


void CallbackSet::remove( PyObject* callback )
{
	if( m_single && safeCmpEqual( m_single.get(), callback ) )
	{
		m_single = 0;
	}
	else if( m_extras )
	{
		safeListDiscard( m_extras.get(), callback );
	}
}


void CallbackSet::dispatch( PyObject* args, PyObject* kwargs )
{
	cppy::ptr list;
	if( m_extras && !( list = copyList( m_extras.get() ) ) )
	{
		printAndClearError();
	}
	if( m_single )
	{
		if( safeIsTruthy( m_single.get() ) )
		{
			safeCallObject( m_single.get(), args, kwargs );
		}
		else
		{
			m_single = 0;
		}
	}
	if( !list )
	{
		return;
	}
	for( Py_ssize_t i = 0, n = PyList_GET_SIZE( list.get() ); i < n; ++i )
	{
		PyObject* cb = PyList_GET_ITEM( list.get(), i );
		if( safeIsTruthy( cb ) )
		{
			safeCallObject( cb, args, kwargs );
		}
		else
		{
			safeListDiscard( m_extras.get(), cb );
		}
	}
}

} // namespace atom
