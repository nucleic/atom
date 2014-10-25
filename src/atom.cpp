/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include "atom.h"
#include "member.h"
#include "methodwrapper.h"
#include "py23compat.h"
#include "utils.h"

#include <algorithm>
#include <cstddef>

#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <pthread.h>
#endif


#define atom_cast( o ) reinterpret_cast<Atom*>( o )
#define member_cast( o ) reinterpret_cast<Member*>( o )
#define signal_cast( o ) reinterpret_cast<Signal*>( o )
#define pymethod_cast( o ) reinterpret_cast<PyMethodObject*>( o )
#define pyobject_cast( o ) reinterpret_cast<PyObject*>( o )


namespace atom
{

namespace
{

typedef Atom::CSVector CSVector;


PyObject* registry;


#ifdef _WIN32
DWORD tls_sender_key;
#else
pthread_key_t tls_sender_key;
#endif


struct CmpLess
{
	template <typename T>
	bool operator()( T& lhs, Signal* rhs )
	{
		return lhs.first < pyobject_cast( rhs );
	}

	template <typename T>
	bool operator()( Signal* lhs, T& rhs )
	{
		return pyobject_cast( lhs ) < rhs.first;
	}
};


struct CmpEqual
{
	template <typename T>
	bool operator()( T& lhs, Signal* rhs )
	{
		return lhs.first == pyobject_cast( rhs );
	}

	template <typename T>
	bool operator()( Signal* lhs, T& rhs )
	{
		return pyobject_cast( lhs ) == rhs.first;
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


inline PyObject* maybeWrapCallback( PyObject* callback )
{
	if( PyMethod_Check( callback ) && PyMethod_GET_SELF( callback ) )
	{
		return MethodWrapper::Create( pymethod_cast( callback ) );
	}
	return cppy::incref( callback );
}


Py_ssize_t getsizeof( CSVector* cbsets )
{
	Py_ssize_t extras = 0;
	typedef CSVector::iterator iter_t;
	for( iter_t it = cbsets->begin(), end = cbsets->end(); it != end; ++it )
	{
		if( it->second.extras() )
		{
			Py_ssize_t size = utils::sys_getsizeof( it->second.extras() );
			if( size < 0 && PyErr_Occurred() )
			{
				return -1;
			}
			extras += size;
		}
	}
	Py_ssize_t vec = static_cast<Py_ssize_t>( sizeof( CSVector ) );
	Py_ssize_t cap = static_cast<Py_ssize_t>( cbsets->capacity() );
	Py_ssize_t val = static_cast<Py_ssize_t>( sizeof( CSVector::value_type ) );
	return vec + cap * val + extras;
}


PyObject* Atom_new( PyTypeObject* type, PyObject* args, PyObject* kwargs )
{
	cppy::ptr members( Atom::LookupMembers( type ) );
	if( !members )
	{
		return 0;
	}
	Py_ssize_t size = PyDict_Size( members.get() );
	cppy::ptr self( type->tp_alloc( type, size ) );
	if( !self )
	{
		return 0;
	}
	Atom* atom = atom_cast( self.get() );
	atom->m_members = members.release();
	return self.release();
}


int Atom_init( PyObject* self, PyObject* args, PyObject* kwargs )
{
	if( PyTuple_GET_SIZE( args ) > 0 )
	{
		cppy::type_error( "__init__() takes no positional arguments" );
		return -1;
	}
	if( kwargs )
	{
		PyObject* key;
		PyObject* value;
		Py_ssize_t pos = 0;
		while( PyDict_Next( kwargs, &pos, &key, &value ) )
		{
			if( PyObject_SetAttr( self, key, value ) < 0 )
			{
				return -1;
			}
		}
	}
	return 0;
}


int Atom_clear( Atom* self )
{
	if( self->m_cbsets )
	{
		CSVector temp; // safe clear
		self->m_cbsets->swap( temp );
	}
	for( Py_ssize_t i = 0, n = Py_SIZE( self ); i < n; ++i )
	{
		Py_CLEAR( self->m_values[ i ] );
	}
	Py_CLEAR( self->m_members );
	return 0;
}


int Atom_traverse( Atom* self, visitproc visit, void* arg )
{
	if( self->m_cbsets )
	{
		typedef CSVector::iterator iter_t;
		iter_t end = self->m_cbsets->end();
		for( iter_t it = self->m_cbsets->begin(); it != end; ++it )
		{
			Py_VISIT( it->first.get() );
			Py_VISIT( it->second.single() );
			Py_VISIT( it->second.extras() );
		}
	}
	for( Py_ssize_t i = 0, n = Py_SIZE( self ); i < n; ++i )
	{
		Py_VISIT( self->m_values[ i ] );
	}
	Py_VISIT( self->m_members );
	return 0;
}


void Atom_dealloc( Atom* self )
{
	PyObject_GC_UnTrack( self );
	if( self->m_weaklist )
	{
		PyObject_ClearWeakRefs( pyobject_cast( self ) );
	}
	Atom_clear( self );
	delete self->m_cbsets;
	Py_TYPE( self )->tp_free( pyobject_cast( self ) );
}


PyObject* Atom_getattro( Atom* self, PyObject* name )
{
	// This is not *strictly* a known-safe cast. While effort is made
	// ensure that the user does not have access to the member registry
	// and hence cannot modify the dict, the GC module will still allow
	// the user to dig into it and add a non-member. My stance is that
	// if they do that, they deserve the segfault. I don't want to pay
	// the extra type checking cost just to protect against a motivated
	// attacker. You can always crash the interpreter with ctypes anyway.
	Member* member = member_cast( PyDict_GetItem( self->m_members, name ) );
	if( member )
	{
		if( member->index() >= Py_SIZE( self ) )
		{
			return cppy::system_error( "invalid member index" );
		}
		cppy::ptr valptr( self->m_values[ member->index() ], true );
		if( valptr )
		{
			return valptr.release();
		}
		valptr = member->defaultValue( pyobject_cast( self ), name );
		if( !valptr )
		{
			return 0;
		}
		self->m_values[ member->index() ] = cppy::incref( valptr.get() );
		return valptr.release();
	}
	return PyObject_GenericGetAttr( pyobject_cast( self ), name );
}


int Atom_setattro( Atom* self, PyObject* name, PyObject* value )
{
	// This is not *strictly* a known-safe cast. While effort is made
	// ensure that the user does not have access to the member registry
	// and hence cannot modify the dict, the GC module will still allow
	// the user to dig into it and add a non-member. My stance is that
	// if they do that, they deserve the segfault. I don't want to pay
	// the extra type checking cost just to protect against a motivated
	// attacker. You can always crash the interpreter with ctypes anyway.
	Member* member = member_cast( PyDict_GetItem( self->m_members, name ) );
	if( member )
	{
		if( member->index() >= Py_SIZE( self ) )
		{
			cppy::system_error( "invalid member index" );
			return -1;
		}
		if( self->m_values[ member->index() ] == value )
		{
			return 0;
		}
		if( !value )
		{
			cppy::clear( &self->m_values[ member->index() ] );
			return 0;
		}
		cppy::ptr valptr( member->validate( pyobject_cast( self ), name, value ) );
		if( !valptr )
		{
			return -1;
		}
		cppy::replace( &self->m_values[ member->index() ], valptr.get() );
		return 0;
	}
	return PyObject_GenericSetAttr( pyobject_cast( self ), name, value );
}


PyObject* Atom_get_member( Atom* self, PyObject* name )
{
	if( !Py23Str_Check( name ) )
	{
		return cppy::type_error( name, "str" );
	}
	PyObject* pyo = PyDict_GetItem( self->m_members, name );
	return cppy::incref( pyo ? pyo : Py_None );
}


PyObject* Atom_get_members( Atom* self, PyObject* args )
{
	return PyDict_Copy( self->m_members );
}


PyObject* Atom_connect( PyObject* ignored, PyObject* args )
{
	PyObject* pyo;
	PyObject* sig;
	PyObject* callback;
	if( !PyArg_UnpackTuple( args, "connect", 3, 3, &pyo, &sig, &callback ) )
	{
		return 0;
	}
	if( !Atom::TypeCheck( pyo ) )
	{
		return cppy::type_error( pyo, "Atom" );
	}
	if( !Signal::TypeCheck( sig ) )
	{
		return cppy::type_error( sig, "Signal" );
	}
	if( !PyCallable_Check( callback ) )
	{
		return cppy::type_error( callback, "callable" );
	}
	return Atom::Connect( atom_cast( pyo ), signal_cast( sig ), callback );
}


PyObject* Atom_disconnect( PyObject* ignored, PyObject* args )
{
	PyObject* pyo;
	PyObject* sig = 0;
	PyObject* callback = 0;
	if( !PyArg_UnpackTuple( args, "disconnect", 1, 3, &pyo, &sig, &callback ) )
	{
		return 0;
	}
	if( !Atom::TypeCheck( pyo ) )
	{
		return cppy::type_error( pyo, "Atom" );
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
		Atom::Disconnect( atom_cast( pyo ) );
	}
	else if( !callback )
	{
		Atom::Disconnect( atom_cast( pyo ), signal_cast( sig ) );
	}
	else
	{
		Atom::Disconnect( atom_cast( pyo ), signal_cast( sig ), callback );
	}
	return cppy::incref( Py_None );
}


PyObject* Atom_emit( PyObject* ignored, PyObject* args, PyObject* kwargs )
{
	Py_ssize_t count = PyTuple_GET_SIZE( args );
	if( count < 2 )
	{
		return cppy::type_error( "Atom.emit() takes at least 2 arguments" );
	}
	PyObject* pyo = PyTuple_GET_ITEM( args, 0 );
	PyObject* sig = PyTuple_GET_ITEM( args, 1 );
	if( !Atom::TypeCheck( pyo ) )
	{
		return cppy::type_error( pyo, "Atom" );
	}
	if( !Signal::TypeCheck( sig ) )
	{
		return cppy::type_error( sig, "Signal" );
	}
	cppy::ptr rest( PyTuple_GetSlice( args, 2, count ) );
	if( !rest )
	{
		return 0;
	}
	Atom::Emit( atom_cast( pyo ), signal_cast( sig ), rest.get(), kwargs );
	return cppy::incref( Py_None );
}


PyObject* Atom_sender( PyObject* mod, PyObject* args )
{
	return Atom::Sender();
}


PyObject* Atom_sizeof( Atom* self, PyObject* args )
{
	Py_ssize_t basic = Py_TYPE( self )->tp_basicsize;
	Py_ssize_t items = Py_SIZE( self ) * sizeof( PyObject* );
	Py_ssize_t cbsets = self->m_cbsets ? getsizeof( self->m_cbsets ) : 0;
	if( cbsets < 0 && PyErr_Occurred() )
	{
		return 0;
	}
	return Py23Int_FromSsize_t( basic + items + cbsets );
}


PyMethodDef Atom_methods[] = {
	{ "get_member",
		( PyCFunction )Atom_get_member,
		METH_O,
		"get_member(name) get the named member for the object or None" },
	{ "get_members",
		( PyCFunction )Atom_get_members,
		METH_NOARGS,
		"get_members() get all of the members for the object as a dict" },
	{ "connect",
		( PyCFunction )Atom_connect,
		METH_VARARGS | METH_STATIC,
		"Atom.connect(atom, signal, callback) connect a signal to a callback" },
	{ "disconnect",
		( PyCFunction )Atom_disconnect,
		METH_VARARGS | METH_STATIC,
		"Atom.disconnect(atom[, signal[, callback]) disconnect a signal from a callback" },
	{ "emit",
		( PyCFunction )Atom_emit,
		METH_VARARGS | METH_KEYWORDS | METH_STATIC,
		"Atom.emit(atom, signal, *args, **kwargs) emit a signal with the given arguments" },
	{ "sender",
		( PyCFunction )Atom_sender,
		METH_NOARGS | METH_STATIC,
		"Atom.sender() get the object emitting the current signal" },
	{ "__sizeof__",
		( PyCFunction )Atom_sizeof,
		METH_NOARGS,
		"__sizeof__() -> size of object in memory, in bytes" },
	{ 0 } // sentinel
};

} // namespace


PyTypeObject Atom::TypeObject = {
	PyVarObject_HEAD_INIT( &PyType_Type, 0 )
	"atom.catom.CAtom",
	sizeof( Atom ) - sizeof( PyObject* ),
	sizeof( PyObject* ),
	( destructor )Atom_dealloc,          /* tp_dealloc */
	( printfunc )0,                      /* tp_print */
	( getattrfunc )0,                    /* tp_getattr */
	( setattrfunc )0,                    /* tp_setattr */
#ifdef IS_PY3K
	( void* )0,                          /* tp_reserved */
#else
	( cmpfunc )0,                        /* tp_compare */
#endif
	( reprfunc )0,                       /* tp_repr */
	( PyNumberMethods* )0,               /* tp_as_number */
	( PySequenceMethods* )0,             /* tp_as_sequence */
	( PyMappingMethods* )0,              /* tp_as_mapping */
	( hashfunc )0,                       /* tp_hash */
	( ternaryfunc )0,                    /* tp_call */
	( reprfunc )0,                       /* tp_str */
	( getattrofunc )Atom_getattro,       /* tp_getattro */
	( setattrofunc )Atom_setattro,       /* tp_setattro */
	( PyBufferProcs* )0,                 /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT
	| Py_TPFLAGS_BASETYPE
	| Py_TPFLAGS_HAVE_GC
	| Py_TPFLAGS_HAVE_VERSION_TAG,       /* tp_flags */
	0,                                   /* Documentation string */
	( traverseproc )Atom_traverse,       /* tp_traverse */
	( inquiry )Atom_clear,               /* tp_clear */
	( richcmpfunc )0,                    /* tp_richcompare */
	offsetof(Atom, m_weaklist),          /* tp_weaklistoffset */
	( getiterfunc )0,                    /* tp_iter */
	( iternextfunc )0,                   /* tp_iternext */
	( struct PyMethodDef* )Atom_methods, /* tp_methods */
	( struct PyMemberDef* )0,            /* tp_members */
	0,                                   /* tp_getset */
	0,                                   /* tp_base */
	0,                                   /* tp_dict */
	( descrgetfunc )0,                   /* tp_descr_get */
	( descrsetfunc )0,                   /* tp_descr_set */
	0,                                   /* tp_dictoffset */
	( initproc )Atom_init,               /* tp_init */
	( allocfunc )PyType_GenericAlloc,    /* tp_alloc */
	( newfunc )Atom_new,                 /* tp_new */
	( freefunc )PyObject_GC_Del,         /* tp_free */
	( inquiry )0,                        /* tp_is_gc */
	0,                                   /* tp_bases */
	0,                                   /* tp_mro */
	0,                                   /* tp_cache */
	0,                                   /* tp_subclasses */
	0,                                   /* tp_weaklist */
	( destructor )0                      /* tp_del */
};


bool Atom::Ready()
{
	if( !( registry = PyDict_New() ) )
	{
		return false;
	}
#ifdef _WIN32
	tls_sender_key = TlsAlloc();
	if( tls_sender_key == TLS_OUT_OF_INDEXES )
	{
		cppy::system_error( "failed to allocate tls key" );
		return false;
	}
#else
	if( pthread_key_create( &tls_sender_key, 0 ) != 0 )
	{
		cppy::system_error( "failed to allocate tls key" );
		return false;
	}
#endif
	return PyType_Ready( &TypeObject ) == 0;
}


bool Atom::RegisterMembers( PyTypeObject* type, PyObject* members )
{
	return PyDict_SetItem( registry, pyobject_cast( type ), members ) == 0;
}


PyObject* Atom::LookupMembers( PyTypeObject* type )
{
	PyObject* members = PyDict_GetItem( registry, pyobject_cast( type ) );
	if( members )
	{
		return cppy::incref( members );
	}
	return cppy::type_error( "type has no registered members" );
}


PyObject* Atom::Sender()
{
#ifdef _WIN32
	void* curr = TlsGetValue( tls_sender_key );
#else
	void* curr = pthread_getspecific( tls_sender_key );
#endif
	return cppy::incref( curr ? pyobject_cast( curr ) : Py_None );
}


PyObject* Atom::Connect( Atom* atom, Signal* sig, PyObject* callback )
{
	cppy::ptr wrapped( maybeWrapCallback( callback ) );
	if( !wrapped )
	{
		return 0;
	}
	if( !atom->m_cbsets )
	{
		atom->m_cbsets = new CSVector();
	}
	cppy::ptr pyptr( pyobject_cast( sig ), true );
	CSVector::iterator it = lowerBound( atom->m_cbsets, sig );
	if( it == atom->m_cbsets->end() || it->first != pyptr )
	{
		CallbackSet cbset( wrapped.get() );
		atom->m_cbsets->insert( it, CSPair( pyptr, cbset ) );
	}
	else
	{
		it->second.add( wrapped.get() );
	}
	return cppy::incref( Py_None );
}


void Atom::Disconnect( Atom* atom )
{
	if( atom->m_cbsets )
	{
		CSVector temp;  // safe clear
		atom->m_cbsets->swap( temp );
	}
}


void Atom::Disconnect( Atom* atom, Signal* sig )
{
	if( atom->m_cbsets )
	{
		CSVector::iterator it = binaryFind( atom->m_cbsets, sig );
		if( it != atom->m_cbsets->end() )
		{
			atom->m_cbsets->erase( it );
		}
	}
}


void Atom::Disconnect( Atom* atom, Signal* sig, PyObject* callback )
{
	if( atom->m_cbsets )
	{
		CSVector::iterator it = binaryFind( atom->m_cbsets, sig );
		if( it != atom->m_cbsets->end() )
		{
			it->second.remove( callback );
		}
	}
}


void Atom::Emit( Atom* atom, Signal* sig, PyObject* args, PyObject* kwargs )
{
	if( atom->m_cbsets )
	{
		CSVector::iterator it = binaryFind( atom->m_cbsets, sig );
		if( it != atom->m_cbsets->end() )
		{
#ifdef _WIN32
			void* prev = TlsGetValue( tls_sender_key );
			TlsSetValue( tls_sender_key, atom );
			it->second.dispatch( args, kwargs );
			TlsSetValue( tls_sender_key, prev );
#else
			void* prev = pthread_getspecific( tls_sender_key );
			pthread_setspecific( tls_sender_key, atom );
			it->second.dispatch( args, kwargs );
			pthread_setspecific( tls_sender_key, prev );
#endif
		}
	}
}

} // namespace atom
