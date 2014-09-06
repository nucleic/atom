/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
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

} // namespace


void CallbackSet::add( PyObject* callback )
{
	if( m_single && safeCmpEqual( m_single.get(), callback ) )
	{
		return;
	}
	if( m_extras && safeSetContains( m_extras.get(), callback ) )
	{
		return;
	}
	if( !m_single )
	{
		m_single = cppy::incref( callback );
		return;
	}
	if( !m_extras && !( m_extras = PySet_New( 0 ) ) )
	{
		printAndClearError();
		return;
	}
	safeSetAdd( m_extras.get(), callback );
}


void CallbackSet::remove( PyObject* callback )
{
	if( m_single && safeCmpEqual( m_single.get(), callback ) )
	{
		m_single = 0;
	}
	else if( m_extras )
	{
		safeSetDiscard( m_extras.get(), callback );
	}
}


void CallbackSet::dispatch( PyObject* args, PyObject* kwargs )
{
	cppy::ptr list;
	if( m_extras && !( list = PySequence_List( m_extras.get() ) ) )
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
	Py_ssize_t n = PyList_GET_SIZE( list.get() );
	for( Py_ssize_t i = 0; i < n; ++i )
	{
		PyObject* cb = PyList_GET_ITEM( list.get(), i );
		if( safeIsTruthy( cb ) )
		{
			safeCallObject( cb, args, kwargs );
		}
		else
		{
			safeSetDiscard( m_extras.get(), cb );
		}
	}
}

} // namespace atom
