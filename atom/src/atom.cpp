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


namespace atom
{

namespace
{

PyObject* members_registry;


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
	Atom* self = reinterpret_cast<Atom*>( self_ptr.get() );
	Py_ssize_t count = PyDict_Size( members_ptr.get() );
	new( &self->m_values ) Atom::ValueVector( count );
	self->m_class_members = members_ptr.release();
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
	Py_CLEAR( self->m_class_members );
	Py_CLEAR( self->m_extra_members );
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
	Py_VISIT( self->m_class_members );
	Py_VISIT( self->m_extra_members );
	return 0;
}


void Atom_dealloc( Atom* self )
{
	PyObject_GC_UnTrack( self );
	Atom_clear( self );
	self->m_values.Atom::ValueVector::~ValueVector();
	self->ob_type->tp_free( reinterpret_cast<PyObject*>( self ) );
}


PyObject* Atom_getattro( Atom* self, PyObject* name )
{
	PyObject* pymember = PyDict_GetItem( self->m_class_members, name );
	if( pymember )
	{
		Member* member = reinterpret_cast<Member*>( pymember );
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
		valptr = member->defaultValue( reinterpret_cast<PyObject*>( self ), name );
		if( !valptr )
		{
			return 0;
		}
		self->m_values[ index ] = valptr;
		return valptr.release();
	}
	return PyObject_GenericGetAttr( reinterpret_cast<PyObject*>( self ), name );
}


int Atom_setattro( Atom* self, PyObject* name, PyObject* value )
{
	PyObject* pymember = PyDict_GetItem( self->m_class_members, name );
	if( pymember )
	{
		Member* member = reinterpret_cast<Member*>( pymember );
		uint32_t index = member->valueIndex();
		if( index >= self->m_values.size() )
		{
			cppy::system_error( "value index out of range" );
			return -1;
		}
		if( !value )
		{
			cppy::type_error( "can't delete atom member" );
			return -1;
		}
		cppy::ptr oldptr( self->m_values[ index ] );
		if( oldptr == value )
		{
			return 0;
		}
		cppy::ptr valptr( member->validateValue( reinterpret_cast<PyObject*>( self ), name, value ) );
		if( !valptr )
		{
			return -1;
		}
		self->m_values[ index ] = valptr;
		return member->postSetAttrValue( reinterpret_cast<PyObject*>( self ), name, valptr.get() );
	}
	return PyObject_GenericSetAttr( reinterpret_cast<PyObject*>( self ), name, value );
}


PyObject* Atom_sizeof( Atom* self, PyObject* args )
{
	Py_ssize_t size = self->ob_type->tp_basicsize;
	size_t capacity = self->m_values.capacity();
	size_t vec_size = capacity * sizeof( Atom::ValueVector::value_type );
	size += static_cast<Py_ssize_t>( vec_size );
	return PyInt_FromSsize_t( size );
}


PyMethodDef Atom_methods[] = {
	{ "__sizeof__",
	  ( PyCFunction )Atom_sizeof,
	  METH_NOARGS,
	  "__sizeof__() -> size of object in memory, in bytes"},
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
	0,                                   /* tp_weaklistoffset */
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
	members_registry = PyDict_New();
	if( !members_registry )
	{
		return false;
	}
	return PyType_Ready( &TypeObject ) == 0;
}


PyObject* Atom::RegisterMembers( PyTypeObject* type, PyObject* members )
{
	if( !PyDict_Check( members ) )
	{
		return cppy::type_error( members, "dict" );
	}
	cppy::ptr members_copy( PyDict_New() );
	if( !members_copy )
	{
		return 0;
	}
	PyObject* key;
	PyObject* value;
	Py_ssize_t pos = 0;
	while( PyDict_Next( members, &pos, &key, &value ) )
	{
		if( !Py23Str_Check( key ) )
		{
			return cppy::type_error( key, "str" );
		}
		if( !Member::TypeCheck( value ) )
		{
			return cppy::type_error( value, "Member" );
		}
		if( PyDict_SetItem( members_copy.get(), key, value ) < 0 )
		{
			return 0;
		}
	}
	PyObject* pyo = reinterpret_cast<PyObject*>( type );
	if( PyDict_SetItem( members_registry, pyo, members_copy.get() ) < 0 )
	{
		return 0;
	}
	return cppy::incref( Py_None );
}


PyObject* Atom::LookupMembers( PyTypeObject* type )
{
	PyObject* pyo = reinterpret_cast<PyObject*>( type );
	PyObject* members = PyDict_GetItem( members_registry, pyo );
	if( members )
	{
		return cppy::incref( members );
	}
	return cppy::type_error( "type has no registered members" );
}

} // namespace atom
