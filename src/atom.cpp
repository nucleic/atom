/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic
|
| Distributed under the terms of the BSD 3-Clause License.
|
| The full license is in the file LICENSE, distributed with this software.
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

#define member_cast( o ) reinterpret_cast<Member*>( o )
#define pymethod_cast( o ) reinterpret_cast<PyMethodObject*>( o )
#define pyobject_cast( o ) reinterpret_cast<PyObject*>( o )


namespace atom
{

namespace
{

typedef Atom::CSVector CSVector;

PyObject* atom_members;

#ifdef _WIN32
DWORD tls_sender_key;
#else
pthread_key_t tls_sender_key;
#endif


inline PyObject* bad_attr_name( PyObject* name )
{
	PyErr_Format(
		PyExc_TypeError,
		"attribute name must be string, not '%.200s'",
		Py_TYPE( name )->tp_name );
	return 0;
}


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
	cppy::ptr members( PyObject_GetAttr( pyobject_cast( type ), atom_members ) );
	if( !members )
	{
		return 0;
	}
	if( !PyDict_Check( members.get() ) )
	{
		return cppy::system_error( "invalid members dict" );
	}
	return type->tp_alloc( type, PyDict_Size( members.get() ) );
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
	if( !Py23Str_Check( name ) )
	{
		return bad_attr_name( name );
	}
	PyObject* pyo = _PyType_Lookup( Py_TYPE( self ), name );
	if( !pyo || !Member::TypeCheck( pyo ) )
	{
		return PyObject_GenericGetAttr( pyobject_cast( self ), name );
	}
	Member* member = member_cast( pyo );
	if( member->index() >= Py_SIZE( self ) )
	{
		return cppy::system_error( "invalid member index" );
	}
	PyObject* value = self->m_values[ member->index() ];
	if( value )
	{
		return cppy::incref( value );
	}
	value = member->defaultValue( pyobject_cast( self ), name );
	if( !value )
	{
		return 0;
	}
	self->m_values[ member->index() ] = value;
	return cppy::incref( value );
}


int Atom_setattro( Atom* self, PyObject* name, PyObject* value )
{
	if( !Py23Str_Check( name ) )
	{
		bad_attr_name( name );
		return -1;
	}
	PyObject* pyo = _PyType_Lookup( Py_TYPE( self ), name );
	if( !pyo || !Member::TypeCheck( pyo ) )
	{
		return PyObject_GenericSetAttr( pyobject_cast( self ), name, value );
	}
	Member* member = member_cast( pyo );
	if( member->index() >= Py_SIZE( self ) )
	{
		cppy::system_error( "invalid member index" );
		return -1;
	}
	if( !value )
	{
		cppy::attribute_error( "can't delete attribute" );
		return -1;
	}
	if( self->m_values[ member->index() ] == value )
	{
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
	if( !( atom_members = Py23Str_FromString( "__atom_members__" ) ) )
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


PyObject* Atom::Sender()
{
#ifdef _WIN32
	void* curr = TlsGetValue( tls_sender_key );
#else
	void* curr = pthread_getspecific( tls_sender_key );
#endif
	return cppy::incref( curr ? pyobject_cast( curr ) : Py_None );
}


bool Atom::Connect( Atom* atom, Signal* sig, PyObject* callback )
{
	cppy::ptr wrapped( maybeWrapCallback( callback ) );
	if( !wrapped )
	{
		return false;
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
	return true;
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
