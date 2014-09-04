/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include "atom.h"
#include "member.h"
#include "py23_compat.h"
#include "stdint.h"


#define atom_cast( o ) reinterpret_cast<Atom*>( o )
#define member_cast( o ) reinterpret_cast<Member*>( o )
#define pyobject_cast( o ) reinterpret_cast<PyObject*>( o )


namespace atom
{

namespace
{

PyObject* registry;


PyObject* Atom_new( PyTypeObject* type, PyObject* args, PyObject* kwargs )
{
	cppy::ptr members_ptr( Atom::LookupMembers( type ) );
	if( !members_ptr )
	{
		return 0;
	}
	cppy::ptr self_ptr( PyType_GenericNew( type, args, kwargs ) );
	if( !self_ptr )
	{
		return 0;
	}
	Atom* self = atom_cast( self_ptr.get() );
	Py_ssize_t count = PyDict_Size( members_ptr.get() );
	new( &self->m_values ) Atom::ValueVector( count );
	self->m_members = members_ptr.release();
	return self_ptr.release();
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
	self->m_values.clear();
	Py_CLEAR( self->m_members );
	return 0;
}


int Atom_traverse( Atom* self, visitproc visit, void* arg )
{
	typedef Atom::ValueVector::iterator iter_t;
	iter_t end = self->m_values.end();
	for( iter_t it = self->m_values.begin(); it != end; ++it )
	{
		Py_VISIT( it->get() );
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
	self->m_values.Atom::ValueVector::~ValueVector();
	self->ob_type->tp_free( pyobject_cast( self ) );
}


PyObject* Atom_getattro( Atom* self, PyObject* name )
{
	if( !Py23Str_Check( name ) )
	{
		return cppy::type_error( "attribute name must be a string" );
	}
	Member* member = member_cast( PyDict_GetItem( self->m_members, name ) );
	if( member )
	{
		uint32_t index = member->valueIndex();
		if( index >= self->m_values.size() )
		{
			return cppy::system_error( "value index out of range" );
		}
		cppy::ptr valptr( self->m_values[ index ] );
		if( valptr )
		{
			return valptr.release();
		}
		valptr = member->defaultValue( pyobject_cast( self ), name );
		if( !valptr )
		{
			return 0;
		}
		self->m_values[ index ] = valptr;
		return valptr.release();
	}
	return PyObject_GenericGetAttr( pyobject_cast( self ), name );
}


int Atom_setattro( Atom* self, PyObject* name, PyObject* value )
{
	if( !Py23Str_Check( name ) )
	{
		cppy::type_error( "attribute name must be a string" );
		return -1;
	}
	Member* member = member_cast( PyDict_GetItem( self->m_members, name ) );
	if( member )
	{
		uint32_t index = member->valueIndex();
		if( index >= self->m_values.size() )
		{
			cppy::system_error( "value index out of range" );
			return -1;
		}
		if( !value )
		{
			self->m_values[ index ] = 0;
			return 0;
		}
		if( self->m_values[ index ] == value )
		{
			return 0;
		}
		cppy::ptr valptr( member->validateValue( pyobject_cast( self ), name, value ) );
		if( !valptr )
		{
			return -1;
		}
		self->m_values[ index ] = valptr;
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


PyObject* Atom_sizeof( Atom* self, PyObject* args )
{
	Py_ssize_t size = self->ob_type->tp_basicsize;
	size_t capacity = self->m_values.capacity();
	size_t vecsize = capacity * sizeof( Atom::ValueVector::value_type );
	return Py23Int_FromSsize_t( size + static_cast<Py_ssize_t>( vecsize ) );
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
	{ "__sizeof__",
	  ( PyCFunction )Atom_sizeof,
	  METH_NOARGS,
	  "__sizeof__() -> size of object in memory, in bytes" },
	{ 0 } // sentinel
};

} // namespace


PyTypeObject Atom::TypeObject = {
	PyObject_HEAD_INIT( &PyType_Type )
	0,                                   /* ob_size */
	"atom.catom.CAtom",                  /* tp_name */
	sizeof( Atom ),                      /* tp_basicsize */
	0,                                   /* tp_itemsize */
	( destructor )Atom_dealloc,          /* tp_dealloc */
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

} // namespace atom
