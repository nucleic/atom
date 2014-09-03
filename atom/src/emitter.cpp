/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include "emitter.h"

#include <algorithm>


namespace atom
{

namespace
{

typedef Emitter::CSVector CSVector;


struct CmpLess
{
	template <typename T>
	bool operator()( T& lhs, Signal* rhs )
	{
		return lhs.first < reinterpret_cast<PyObject*>( rhs );
	}

	template <typename T>
	bool operator()( Signal* lhs, T& rhs )
	{
		return reinterpret_cast<PyObject*>( lhs ) < rhs.first;
	}
};


struct CmpEqual
{
	template <typename T>
	bool operator()( T& lhs, Signal* rhs )
	{
		return lhs.first == reinterpret_cast<PyObject*>( rhs );
	}

	template <typename T>
	bool operator()( Signal* lhs, T& rhs )
	{
		return reinterpret_cast<PyObject*>( lhs ) == rhs.first;
	}
};


inline CSVector::iterator lowerBound( CSVector* cbsets, Signal* sig )
{
	return std::lower_bound( cbsets->begin(), cbsets->end(), sig, CmpLess() );
}


inline CSVector::iterator binaryFind( CSVector* cbsets, Signal* sig )
{
	CSVector::iterator it = lowerBound( cbsets, sig );
	if( it != cbsets->end() && CmpEqual()( *it, sig ) )
	{
		return it;
	}
	return cbsets->end();
}


int Emitter_clear( Emitter* self )
{
	if( self->m_cbsets )
	{
		self->m_cbsets->clear();
	}
	return 0;
}


int Emitter_traverse( Emitter* self, visitproc visit, void* arg )
{
	if( self->m_cbsets )
	{
		typedef CSVector::iterator iter_t;
		iter_t end = self->m_cbsets->end();
		for( iter_t it = self->m_cbsets->begin(); it != end; ++it )
		{
			Py_VISIT( it->first.get() );
			if( int ret = it->second.traverse( visit, arg ) )
			{
				return ret;
			}
		}
	}
	return 0;
}


void Emitter_dealloc( Emitter* self )
{
	PyObject_GC_UnTrack( self );
	delete self->m_cbsets;
	self->ob_type->tp_free( reinterpret_cast<PyObject*>( self ) );
}


PyObject* Emitter_connect( Emitter* self, PyObject* args )
{
	PyObject* sig;
	PyObject* callback;
	if( !PyArg_ParseTuple( args, "OO", &sig, &callback ) )
	{
		return 0;
	}
	if( !Signal::TypeCheck( sig ) )
	{
		return cppy::type_error( sig, "Signal" );
	}
	if( !PyCallable_Check( callback ) )
	{
		return cppy::type_error( callback, "callable" );
	}
	// TODO support weak methods
	self->connect( reinterpret_cast<Signal*>( sig ), callback );
	return cppy::incref( Py_None );
}


PyObject* Emitter_disconnect( Emitter* self, PyObject* args )
{
	PyObject* sig = 0;
	PyObject* callback = 0;
	if( !PyArg_ParseTuple( args, "|OO", &sig, &callback ) )
	{
		return 0;
	}
	if( sig && !Signal::TypeCheck( sig ) )
	{
		return cppy::type_error( sig, "Signal" );
	}
	if( callback && !PyCallable_Check( callback ) )
	{
		return cppy::type_error( callback, "callable" );
	}
	if( !sig )
	{
		self->disconnect();
	}
	else if( !callback )
	{
		self->disconnect( reinterpret_cast<Signal*>( sig ) );
	}
	else
	{
		// TODO support weak methods
		self->disconnect( reinterpret_cast<Signal*>( sig ), callback );
	}
	return cppy::incref( Py_None );
}


PyObject* Emitter_emit( Emitter* self, PyObject* args, PyObject* kwargs )
{
	Py_ssize_t arg_count = PyTuple_GET_SIZE( args );
	if( arg_count == 0 )
	{
		return cppy::type_error( "emit() takes at least 1 argument (0 given)" );
	}
	PyObject* sig = PyTuple_GET_ITEM( args, 0 );
	if( !Signal::TypeCheck( sig ) )
	{
		return cppy::type_error( sig, "Signal" );
	}
	cppy::ptr rest( PyTuple_GetSlice( args, 1, arg_count ) );
	// TODO push to a sender stack
	self->emit( reinterpret_cast<Signal*>( sig ), rest.get(), kwargs );
	// TODO pop from a sender stack
	return cppy::incref( Py_None );
}


PyMethodDef Emitter_methods[] = {
	{ "connect",
	  ( PyCFunction )Emitter_connect,
	  METH_VARARGS,
	  "connect(signal, callback) connect a signal to a callback" },
	{ "disconnect",
	  ( PyCFunction )Emitter_disconnect,
	  METH_VARARGS,
	  "disconnect([signal[, callback]) disconnect a signal from a callback" },
	{ "emit",
	  ( PyCFunction )Emitter_emit,
	  METH_VARARGS | METH_KEYWORDS,
	  "emit(signal, *args, **kwargs) emit a signal with the given arguments" },
	{ 0 } // sentinel
};

} // namespace


PyTypeObject Emitter::TypeObject = {
	PyObject_HEAD_INIT( &PyType_Type )
	0,                                   /* ob_size */
	"atom.catom.Emitter",                /* tp_name */
	sizeof( Emitter ),                   /* tp_basicsize */
	0,                                   /* tp_itemsize */
	( destructor )Emitter_dealloc,       /* tp_dealloc */
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
	( getattrofunc )0,                   /* tp_getattro */
	( setattrofunc )0,                   /* tp_setattro */
	( PyBufferProcs* )0,                 /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT
	| Py_TPFLAGS_BASETYPE
	| Py_TPFLAGS_HAVE_GC
	| Py_TPFLAGS_HAVE_VERSION_TAG,       /* tp_flags */
	0,                                   /* Documentation string */
	( traverseproc )Emitter_traverse,    /* tp_traverse */
	( inquiry )Emitter_clear,            /* tp_clear */
	( richcmpfunc )0,                    /* tp_richcompare */
	0,                                   /* tp_weaklistoffset */
	( getiterfunc )0,                    /* tp_iter */
	( iternextfunc )0,                   /* tp_iternext */
	( struct PyMethodDef* )Emitter_methods, /* tp_methods */
	( struct PyMemberDef* )0,            /* tp_members */
	0,                                   /* tp_getset */
	0,                                   /* tp_base */
	0,                                   /* tp_dict */
	( descrgetfunc )0,                   /* tp_descr_get */
	( descrsetfunc )0,                   /* tp_descr_set */
	0,                                   /* tp_dictoffset */
	( initproc )0,                       /* tp_init */
	( allocfunc )PyType_GenericAlloc,    /* tp_alloc */
	( newfunc )PyType_GenericNew,        /* tp_new */
	( freefunc )PyObject_GC_Del,         /* tp_free */
	( inquiry )0,                        /* tp_is_gc */
	0,                                   /* tp_bases */
	0,                                   /* tp_mro */
	0,                                   /* tp_cache */
	0,                                   /* tp_subclasses */
	0,                                   /* tp_weaklist */
	( destructor )0                      /* tp_del */
};


bool Emitter::Ready()
{
	return PyType_Ready( &TypeObject ) == 0;
}


void Emitter::connect( Signal* sig, PyObject* callback )
{
	if( !m_cbsets )
	{
		m_cbsets = new CSVector();
	}
	CSVector::iterator it = lowerBound( m_cbsets, sig );
	PyObject* pyo = reinterpret_cast<PyObject*>( sig );
	if( it == m_cbsets->end() || it->first != pyo )
	{
		cppy::ptr pyoptr( pyo, true );
		CallbackSet cbset( callback );
		m_cbsets->insert( it, CSPair( pyoptr, cbset ) );
	}
	else
	{
		it->second.add( callback );
	}
}


void Emitter::disconnect()
{
	if( m_cbsets )
	{
		m_cbsets->clear();
	}
}


void Emitter::disconnect( Signal* sig )
{
	if( m_cbsets )
	{
		CSVector::iterator it = binaryFind( m_cbsets, sig );
		if( it != m_cbsets->end() )
		{
			m_cbsets->erase( it );
		}
	}
}


void Emitter::disconnect( Signal* sig, PyObject* callback )
{
	if( m_cbsets )
	{
		CSVector::iterator it = binaryFind( m_cbsets, sig );
		if( it != m_cbsets->end() )
		{
			it->second.remove( callback );
		}
	}
}


void Emitter::emit( Signal* sig, PyObject* args, PyObject* kwargs )
{
	if( m_cbsets )
	{
		CSVector::iterator it = binaryFind( m_cbsets, sig );
		if( it != m_cbsets->end() )
		{
			it->second.dispatch( args, kwargs );
		}
	}
}

} // namespace atom
