/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include "dispatcher.h"


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


inline bool safeDispatch( PyObject* cb, PyObject* args, PyObject* kwargs )
{
	if( !safeIsTruthy( cb ) )
	{
		return false;
	}
	cppy::ptr result( PyObject_Call( cb, args, kwargs ) );
	if( !result )
	{
		printAndClearError();
		return false;
	}
	return true;
}

} // namespace


void CallbackSet::add( PyObject* callback )
{
	if( m_single && safeCmpEqual( m_single.get(), callback ) )
	{
		return;
	}
	if( m_set && safeSetContains( m_set.get(), callback ) )
	{
		return;
	}
	if( !m_single )
	{
		m_single = cppy::incref( callback );
		return;
	}
	if( !m_set && !( m_set = PySet_New( 0 ) ) )
	{
		printAndClearError();
		return;
	}
	safeSetAdd( m_set.get(), callback );
}


void CallbackSet::remove( PyObject* callback )
{
	if( m_single && safeCmpEqual( m_single.get(), callback ) )
	{
		m_single = 0;
		return;
	}
	if( m_set )
	{
		safeSetDiscard( m_set.get(), callback );
		return;
	}
}


void CallbackSet::dispatch( PyObject* args, PyObject* kwargs )
{
	cppy::ptr cblist;
	if( m_set && !( cblist = PySequence_List( m_set.get() ) ) )
	{
		printAndClearError();
	}
	if( m_single && !safeDispatch( m_single.get(), args, kwargs ) )
	{
		m_single = 0;
	}
	if( !cblist )
	{
		return;
	}
	Py_ssize_t n = PyList_GET_SIZE( cblist.get() );
	for( Py_ssize_t i = 0; i < n; ++i )
	{
		PyObject* cb = PyList_GET_ITEM( cblist.get(), i );
		if( !safeDispatch( cb, args, kwargs ) )
		{
			safeSetDiscard( m_set.get(), cb );
		}
	}
}


void Dispatcher::connect( PyObject* name, PyObject* callback )
{
	CBSVector::iterator it = find( name );
	if( it == m_cbsets.end() )
	{
		cppy::ptr nameptr( name, true );
		CallbackSet cbset( callback );
		m_cbsets.push_back( CBSPair( nameptr, cbset ) );
	}
	else
	{
		it->second.add( callback );
	}
}


void Dispatcher::disconnect()
{
	m_cbsets.clear();
}


void Dispatcher::disconnect( PyObject* name )
{
	CBSVector::iterator it = find( name );
	if( it != m_cbsets.end() )
	{
		m_cbsets.erase( it );
	}
}


void Dispatcher::disconnect( PyObject* name, PyObject* callback )
{
	CBSVector::iterator it = find( name );
	if( it != m_cbsets.end() )
	{
		it->second.remove( callback );
	}
}


void Dispatcher::emit( PyObject* name, PyObject* args, PyObject* kwargs )
{
	CBSVector::iterator it = find( name );
	if( it != m_cbsets.end() )
	{
		it->second.dispatch( args, kwargs );
	}
}


Dispatcher::CBSVector::iterator Dispatcher::find( PyObject* name )
{
	typedef CBSVector::iterator iter_t;
	for( iter_t it = m_cbsets.begin(); it < m_cbsets.end(); ++it )
	{
		if( safeCmpEqual( it->first.get(), name ) )
		{
			return it;
		}
	}
	return m_cbsets.end();
}

} // namespace atom
