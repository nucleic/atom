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

#include <cppy/cppy.h>
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

PyObject* members_str;

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


inline PyObject* maybe_wrap_callback( PyObject* callback )
{
	if( PyMethod_Check( callback ) && PyMethod_GET_SELF( callback ) )
	{
		return MethodWrapper::Create( pymethod_cast( callback ) );
	}
	return cppy::incref( callback );
}


inline Py_ssize_t list_index( PyObject* list, PyObject* value )
{
	for( Py_ssize_t i = 0; i < PyList_GET_SIZE( list ); ++i )
	{
		int ok = PyObject_RichCompareBool( PyList_GET_ITEM( list, i ), value, Py_EQ );
		if( ok == -1 )
		{
			return -1;
		}
		if( ok == 1 )
		{
			return i;
		}
	}
	return -1;
}


inline bool list_add( PyObject* list, PyObject* value )
{
	Py_ssize_t index = list_index( list, value );
	if( index != -1 )
	{
		return true;
	}
	if( PyErr_Occurred() )
	{
		return false;
	}
	return PyList_Append( list, value ) == 0;
}


inline bool list_discard( PyObject* list, PyObject* value )
{
	Py_ssize_t index = list_index( list, value );
	if( index != -1 )
	{
		return PyList_SetSlice( list, index, index + 1, 0 ) == 0;
	}
	return !PyErr_Occurred();
}


inline void safe_list_discard( PyObject* list, PyObject* value )
{
	if( !list_discard( list, value ) && PyErr_Occurred() )
	{
		PyErr_Print();
	}
}


inline bool safe_is_truthy( PyObject* ob )
{
	int ok = PyObject_IsTrue( ob );
	if( ok == 1 )
	{
		return true;
	}
	if( ok == 0 )
	{
		return false;
	}
	PyErr_Print();
	return false;
}


inline void safe_call_object( PyObject* ob, PyObject* args, PyObject* kwargs )
{
	cppy::ptr ok( PyObject_Call( ob, args, kwargs ) );
	if( !ok )
	{
		PyErr_Print();
	}
}


bool connect( Atom* atom, PyObject* name, PyObject* callback )
{
	cppy::ptr cb( maybe_wrap_callback( callback ) );
	if( !cb )
	{
		return false;
	}
	if( !atom->m_callbacks && !( atom->m_callbacks = PyDict_New() ) )
	{
		return false;
	}
	PyObject* current = PyDict_GetItem( atom->m_callbacks, name );
	if( !current )
	{
		return PyDict_SetItem( atom->m_callbacks, name, cb.get() ) == 0;
	}
	if( PyList_Check( current ) )
	{
		return list_add( current, cb.get() );
	}
	int ok = PyObject_RichCompareBool( current, cb.get(), Py_EQ );
	if( ok == -1 )
	{
		return false;
	}
	if( ok == 1 )
	{
		return true;
	}
	cppy::ptr list( PyList_New( 2 ) );
	if( !list )
	{
		return false;
	}
	PyList_SET_ITEM( list.get(), 0, cppy::incref( current ) );
	PyList_SET_ITEM( list.get(), 1, cb.release() );
	return PyDict_SetItem( atom->m_callbacks, name, list.get() ) == 0;
}


bool disconnect( Atom* atom, PyObject* name = 0, PyObject* callback = 0 )
{
	if( !atom->m_callbacks )
	{
		return true;
	}
	if( !name )
	{
		cppy::clear( &atom->m_callbacks );
		return true;
	}
	PyObject* current = PyDict_GetItem( atom->m_callbacks, name );
	if( !current )
	{
		return true;
	}
	if( !callback )
	{
		return PyDict_DelItem( atom->m_callbacks, name ) == 0;
	}
	if( PyList_Check( current ) )
	{
		return list_discard( current, callback );
	}
	int ok = PyObject_RichCompareBool( current, callback, Py_EQ );
	if( ok == -1 )
	{
		return false;
	}
	if( ok == 0 )
	{
		return true;
	}
	return PyDict_DelItem( atom->m_callbacks, name ) == 0;
}


void dispatch( PyObject* cbs, PyObject* args, PyObject* kwargs )
{
	if( !PyList_Check( cbs ) )
	{
		safe_call_object( cbs, args, kwargs );
		return;
	}
	cppy::ptr list( PyList_GetSlice( cbs, 0, PyList_GET_SIZE( cbs ) ) );
	if( !list )
	{
		PyErr_Print();
		return;
	}
	for( Py_ssize_t i = 0, n = PyList_GET_SIZE( list.get() ); i < n; ++i )
	{
		PyObject* cb = PyList_GET_ITEM( list.get(), i );
		if( safe_is_truthy( cb ) )
		{
			safe_call_object( cb, args, kwargs );
		}
		else
		{
			safe_list_discard( cbs, cb );
		}
	}
}


bool emit( Atom* atom, PyObject* name, PyObject* args, PyObject* kwargs )
{
	if( !atom->m_callbacks )
	{
		return true;
	}
	PyObject* cbs = PyDict_GetItem( atom->m_callbacks, name );
	if( !cbs )
	{
		return true;
	}
	int ok = PyObject_IsTrue( cbs );
	if( ok == -1 )
	{
		return false;
	}
	if( ok == 0 )
	{
		return PyDict_DelItem( atom->m_callbacks, name ) == 0;
	}
#ifdef _WIN32
	void* prev = TlsGetValue( tls_sender_key );
	TlsSetValue( tls_sender_key, atom );
	dispatch( cbs, args, kwargs );
	TlsSetValue( tls_sender_key, prev );
#else
	void* prev = pthread_getspecific( tls_sender_key );
	pthread_setspecific( tls_sender_key, atom );
	dispatch( cbs, args, kwargs );
	pthread_setspecific( tls_sender_key, prev );
#endif
	return true;
}


PyObject* Atom_new( PyTypeObject* type, PyObject* args, PyObject* kwargs )
{
	cppy::ptr members( PyObject_GetAttr( pyobject_cast( type ), members_str ) );
	if( !members )
	{
		return 0;
	}
	if( !PyTuple_Check( members.get() ) )
	{
		return cppy::system_error( "invalid members tuple" );
	}
	return type->tp_alloc( type, PyTuple_GET_SIZE( members.get() ) );
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
	Py_CLEAR( self->m_callbacks );
	for( Py_ssize_t i = 0, n = Py_SIZE( self ); i < n; ++i )
	{
		Py_CLEAR( self->m_values[ i ] );
	}
	return 0;
}


int Atom_traverse( Atom* self, visitproc visit, void* arg )
{
	Py_VISIT( self->m_callbacks );
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
	PyObject* atom = pyobject_cast( self );
	cppy::ptr valptr( member->defaultv( atom, name ) );
	if( !valptr )
	{
		return 0;
	}
	valptr = member->validate( atom, name, valptr.get() );
	if( !valptr )
	{
		return 0;
	}
	valptr = member->post_validate( atom, name, valptr.get() );
	if( !valptr )
	{
		return 0;
	}
	cppy::replace( &self->m_values[ member->index() ], valptr.get() );
	return valptr.release();
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
	cppy::ptr oldptr( self->m_values[ member->index() ], true );
	if( oldptr == value )
	{
		return 0;
	}
	PyObject* atom = pyobject_cast( self );
	cppy::ptr valptr( member->validate( atom, name, value ) );
	if( !valptr )
	{
		return -1;
	}
	valptr = member->post_validate( atom, name, valptr.get() );
	if( !valptr )
	{
		return -1;
	}
	cppy::replace( &self->m_values[ member->index() ], valptr.get() );
	if( !oldptr )
	{
		oldptr = cppy::incref( Py_None );
	}
	if( member->post_setattr( atom, name, oldptr.get(), valptr.get() ) < 0 )
	{
		return -1;
	}
	return 0;
}


PyObject* Atom_sender( PyObject* ig1, PyObject* ig2 )
{
#ifdef _WIN32
	void* curr = TlsGetValue( tls_sender_key );
#else
	void* curr = pthread_getspecific( tls_sender_key );
#endif
	return cppy::incref( curr ? pyobject_cast( curr ) : Py_None );
}


PyObject* Atom_connect( Atom* self, PyObject* args )
{
	PyObject* name;
	PyObject* callback;
	if( !PyArg_UnpackTuple( args, "connect", 2, 2, &name, &callback ) )
	{
		return 0;
	}
	if( !Py23Str_Check( name ) )
	{
		return cppy::type_error( name, "str" );
	}
	if( !PyCallable_Check( callback ) )
	{
		return cppy::type_error( callback, "callable" );
	}
	if( !connect( self, name, callback ) )
	{
		return 0;
	}
	return cppy::incref( Py_None );
}


PyObject* Atom_disconnect( Atom* self, PyObject* args )
{
	PyObject* name = 0;
	PyObject* callback = 0;
	if( !PyArg_UnpackTuple( args, "disconnect", 0, 2, &name, &callback ) )
	{
		return 0;
	}
	if( name && !Py23Str_Check( name ) )
	{
		return cppy::type_error( name, "str" );
	}
	if( callback && !PyCallable_Check( callback ) )
	{
		return cppy::type_error( callback, "callable" );
	}
	if( !disconnect( self, name, callback ) )
	{
		return 0;
	}
	return cppy::incref( Py_None );
}


PyObject* Atom_emit( Atom* self, PyObject* args, PyObject* kwargs )
{
	Py_ssize_t count = PyTuple_GET_SIZE( args );
	if( count == 0 )
	{
		return cppy::type_error( "emit() takes at least 1 argument (0 given)" );
	}
	PyObject* name = PyTuple_GET_ITEM( args, 0 );
	if( !Py23Str_Check( name ) )
	{
		return cppy::type_error( name, "str" );
	}
	cppy::ptr rest( count == 1 ? PyTuple_New( 0 ) : PyTuple_GetSlice( args, 1, count ) );
	if( !rest )
	{
		return 0;
	}
	if( !emit( self, name, rest.get(), kwargs ) )
	{
		return 0;
	}
	return cppy::incref( Py_None );
}


PyObject* Atom_sizeof( Atom* self, PyObject* args )
{
	Py_ssize_t basic = Py_TYPE( self )->tp_basicsize;
	Py_ssize_t items = Py_SIZE( self ) * sizeof( PyObject* );
	return Py23Int_FromSsize_t( basic + items );
}


PyMethodDef Atom_methods[] = {
	{ "sender",
		( PyCFunction )Atom_sender,
		METH_NOARGS | METH_STATIC,
		"sender() [staticmethod] get the current signal emitter" },
	{ "connect",
		( PyCFunction )Atom_connect,
		METH_VARARGS,
		"connect(name, callback) connect a signal to a callback" },
	{ "disconnect",
		( PyCFunction )Atom_disconnect,
		METH_VARARGS,
		"disconnect([name[, callback]) disconnect a signal from a callback" },
	{ "emit",
		( PyCFunction )Atom_emit,
		METH_VARARGS | METH_KEYWORDS,
		"emit(name, *args, **kwargs) emit a signal with the given arguments" },
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
	if( !( members_str = Py23Str_FromString( "__members__" ) ) )
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

} // namespace atom
