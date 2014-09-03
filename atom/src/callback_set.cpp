/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include "callback_set.h"
#include "safe_utils.h"


namespace atom
{

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
	}
	else if( m_set )
	{
		safeSetDiscard( m_set.get(), callback );
	}
}


void CallbackSet::dispatch( PyObject* args, PyObject* kwargs )
{
	cppy::ptr list;
	if( m_set && !( list = PySequence_List( m_set.get() ) ) )
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
			safeSetDiscard( m_set.get(), cb );
		}
	}
}


int CallbackSet::traverse( visitproc visit, void* arg )
{
	Py_VISIT( m_single.get() );
	Py_VISIT( m_set.get() );
}

} // namespace atom
