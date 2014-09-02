/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include "dispatcher.h"
#include "safe_utils.h"

#include <algorithm>


namespace atom
{

namespace
{

// Both of the operator versions are needed in order to keep the
// MSVC debug version of std::lower_bound happy, as it performs
// reflexive checks to ensure strict weak ordering.
struct CmpLess
{
	template <typename T>
	bool operator()( T& lhs, PyObject* rhs )
	{
		return safeCmpLess( lhs.first.get(), rhs );
	}

	template <typename T>
	bool operator()( PyObject* lhs, T& rhs )
	{
		return safeCmpLess( lhs, rhs.first.get() );
	}
};

} // namespace


void Dispatcher::connect( PyObject* name, PyObject* callback )
{
	CBSVector::iterator it = lowerBound( name );
	if( it == m_cbsets.end() || !safeCmpEqual( it->first.get(), name ) )
	{
		cppy::ptr nameptr( name, true );
		CallbackSet cbset( callback );
		m_cbsets.insert( it, CBSPair( nameptr, cbset ) );
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
	CBSVector::iterator it = lowerBound( name );
	if( it != m_cbsets.end() && safeCmpEqual( it->first.get(), name ) )
	{
		return it;
	}
	return m_cbsets.end();
}


Dispatcher::CBSVector::iterator Dispatcher::lowerBound( PyObject* name )
{
	return std::lower_bound( m_cbsets.begin(), m_cbsets.end(), name, CmpLess() );
}


struct PyDispatcher
{
	PyObject_HEAD
	Dispatcher* m_dispatcher;
};


static PyObject* PyDispatcher_new( PyTypeObject* type, PyObject* args, PyObject* kwargs )
{
	PyObject* py = PyType_GenericNew( type, args, kwargs );
	reinterpret_cast<PyDispatcher*>( py )->m_dispatcher = new Dispatcher();
	return py;
};


static void PyDispatcher_dealloc( PyDispatcher* self )
{
	delete self->m_dispatcher;
	self->ob_type->tp_free( reinterpret_cast<PyObject*>( self ) );
}


static PyObject* PyDispatcher_connect( PyDispatcher* self, PyObject* args )
{
	PyObject* name;
	PyObject* callback;
	if( !PyArg_ParseTuple( args, "OO", &name, &callback ) )
	{
		return 0;
	}
	self->m_dispatcher->connect( name, callback );
	return cppy::incref( Py_None );
}


static PyObject* PyDispatcher_disconnect( PyDispatcher* self, PyObject* args )
{
	PyObject* name;
	PyObject* callback;
	if( !PyArg_ParseTuple( args, "OO", &name, &callback ) )
	{
		return 0;
	}
	self->m_dispatcher->disconnect( name, callback );
	return cppy::incref( Py_None );
}


static PyObject* PyDispatcher_emit( PyDispatcher* self, PyObject* args, PyObject* kwargs )
{
	PyObject* name = PyTuple_GET_ITEM( args, 0 );
	cppy::ptr nargs( PyTuple_GetSlice( args, 1, PyTuple_GET_SIZE( args ) ) );
	self->m_dispatcher->emit( name, nargs.get(), kwargs );
	return cppy::incref( Py_None );
}


PyMethodDef PyDispatcher_methods[] = {
	{ "connect",
	  ( PyCFunction )PyDispatcher_connect,
	  METH_VARARGS,
	  "connect" },
	{ "disconnect",
	  ( PyCFunction )PyDispatcher_disconnect,
	  METH_VARARGS,
	  "disconnect" },
	{ "emit",
	  ( PyCFunction )PyDispatcher_emit,
	  METH_KEYWORDS,
	  "emit" },
	{ 0 } // sentinel
};


PyTypeObject Dispatcher::TypeObject = {
	PyObject_HEAD_INIT( &PyType_Type )
	0,                                   /* ob_size */
	"atom.catom.PyDispatcher",                  /* tp_name */
	sizeof( PyDispatcher ),                      /* tp_basicsize */
	0,                                   /* tp_itemsize */
	( destructor )PyDispatcher_dealloc,          /* tp_dealloc */
	( printfunc )0,                      /* tp_print */
	( getattrfunc )0,                    /* tp_getattr */
	( setattrfunc )0,                    /* tp_setattr */
	( cmpfunc )0,                        /* tp_compare */
	( reprfunc )0,                       /* tp_repr */
	( PyNumberMethods* )0,               /* tp_as_number */
	( PySequenceMethods* )0,             /* tp_as_sequence */
	( PyMappingMethods* )0,              /* tp_as_mapping */
	( hashfunc )0,                       /* tp_hash */
	( ternaryfunc )0,                    /* tp_call */
	( reprfunc )0,                       /* tp_str */
	( getattrofunc )0,       /* tp_getattro */
	( setattrofunc )0,       /* tp_setattro */
	( PyBufferProcs* )0,                 /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT
	| Py_TPFLAGS_BASETYPE
	| Py_TPFLAGS_HAVE_VERSION_TAG,       /* tp_flags */
	0,                                   /* Documentation string */
	( traverseproc )0,       /* tp_traverse */
	( inquiry )0,               /* tp_clear */
	( richcmpfunc )0,                    /* tp_richcompare */
	0,       /* tp_weaklistoffset */
	( getiterfunc )0,                    /* tp_iter */
	( iternextfunc )0,                   /* tp_iternext */
	( struct PyMethodDef* )PyDispatcher_methods, /* tp_methods */
	( struct PyMemberDef* )0,            /* tp_members */
	0,                                   /* tp_getset */
	0,                                   /* tp_base */
	0,                                   /* tp_dict */
	( descrgetfunc )0,                   /* tp_descr_get */
	( descrsetfunc )0,                   /* tp_descr_set */
	0,                                   /* tp_dictoffset */
	( initproc )0,               /* tp_init */
	( allocfunc )PyType_GenericAlloc,    /* tp_alloc */
	( newfunc )PyDispatcher_new,                 /* tp_new */
	( freefunc )PyObject_Free,         /* tp_free */
	( inquiry )0,                        /* tp_is_gc */
	0,                                   /* tp_bases */
	0,                                   /* tp_mro */
	0,                                   /* tp_cache */
	0,                                   /* tp_subclasses */
	0,                                   /* tp_weaklist */
	( destructor )0                      /* tp_del */
};


bool Dispatcher::Ready()
{
	return PyType_Ready( &TypeObject ) == 0;
}

} // namespace atom
