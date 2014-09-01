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

PyObject* class_members_registry;


// returns borrowed reference to Member or null - no-except
inline Member* lookup_member( Atom* atom, PyObject* name )
{
	PyObject* member = 0;
	if( atom->m_extra_members )
	{
		member = PyDict_GetItem( atom->m_extra_members, name );
	}
	if( !member )
	{
		member = PyDict_GetItem( atom->m_class_members, name );
	}
	return reinterpret_cast<Member*>( member );
}


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
	if( self->m_weakreflist )
	{
		PyObject_ClearWeakRefs( reinterpret_cast<PyObject*>( self ) );
	}
	Atom_clear( self );
	self->m_values.Atom::ValueVector::~ValueVector();
	self->ob_type->tp_free( reinterpret_cast<PyObject*>( self ) );
}


PyObject* Atom_getattro( Atom* self, PyObject* name )
{
	if( !Py23Str_Check( name ) )
	{
		return cppy::type_error( "attribute name must be a string" );
	}
	Member* member = lookup_member( self, name );
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
	if( !Py23Str_Check( name ) )
	{
		cppy::type_error( "attribute name must be a string" );
		return -1;
	}
	Member* member = lookup_member( self, name );
	if( member )
	{
		uint32_t index = member->valueIndex();
		if( index >= self->m_values.size() )
		{
			cppy::system_error( "value index out of range" );
			return -1;
		}
		if( self->m_values[ index ] == value )
		{
			return 0;
		}
		if( !value )
		{
			self->m_values[ index ] = 0;
			return 0;
		}
		cppy::ptr valptr( member->validateValue( reinterpret_cast<PyObject*>( self ), name, value ) );
		if( !valptr )
		{
			return -1;
		}
		self->m_values[ index ] = valptr;
		return 0;
	}
	return PyObject_GenericSetAttr( reinterpret_cast<PyObject*>( self ), name, value );
}


PyObject* Atom_get_member( Atom* self, PyObject* name )
{
	if( !Py23Str_Check( name ) )
	{
		return cppy::type_error( name, "str" );
	}
	Member* member = lookup_member( self, name );
	PyObject* pyo = reinterpret_cast<PyObject*>( member );
	return cppy::incref( pyo ? pyo : Py_None );
}


PyObject* Atom_get_class_member( Atom* self, PyObject* name )
{
	if( !Py23Str_Check( name ) )
	{
		return cppy::type_error( name, "str" );
	}
	PyObject* pyo = PyDict_GetItem( self->m_class_members, name );
	return cppy::incref( pyo ? pyo : Py_None );
}


PyObject* Atom_get_extra_member( Atom* self, PyObject* name )
{
	if( !Py23Str_Check( name ) )
	{
		return cppy::type_error( name, "str" );
	}
	if( !self->m_extra_members )
	{
		return cppy::incref( Py_None );
	}
	PyObject* pyo = PyDict_GetItem( self->m_extra_members, name );
	return cppy::incref( pyo ? pyo : Py_None );
}


PyObject* Atom_get_members( Atom* self, PyObject* args )
{
	cppy::ptr result( PyDict_Copy( self->m_class_members ) );
	if( !result )
	{
		return 0;
	}
	if( self->m_extra_members )
	{
		if( PyDict_Update( result.get(), self->m_extra_members ) < 0 )
		{
			return 0;
		}
	}
	return result.release();
}


PyObject* Atom_get_class_members( Atom* self, PyObject* args )
{
	return PyDict_Copy( self->m_class_members );
}


PyObject* Atom_get_extra_members( Atom* self, PyObject* args )
{
	if( self->m_extra_members )
	{
		return PyDict_Copy( self->m_extra_members );
	}
	return PyDict_New();
}


PyObject* Atom_add_extra_member( Atom* self, PyObject* args )
{
	PyObject* name;
	PyObject* pyo;
	PyObject* value = 0;
	if( !PyArg_ParseTuple( args, "OO|O", &name, &pyo, &value ) )
	{
		return 0;
	}
	if( !Py23Str_Check( name ) )
	{
		return cppy::type_error( name, "str" );
	}
	if( !Member::TypeCheck( pyo ) )
	{
		return cppy::type_error( pyo, "Member" );
	}
	cppy::ptr clone( Member::Clone( pyo ) );
	if( !clone )
	{
		return 0;
	}
	Member* existing = lookup_member( self, name );
	if( !self->m_extra_members && !( self->m_extra_members = PyDict_New() ) )
	{
		return 0;
	}
	if( PyDict_SetItem( self->m_extra_members, name, clone.get() ) < 0 )
	{
		return 0;
	}
	Member* member = reinterpret_cast<Member*>( clone.get() );
	if( existing )
	{
		member->setValueIndex( existing->valueIndex() );
		self->m_values[ member->valueIndex() ] = 0;
	}
	else
	{
		member->setValueIndex( static_cast<uint16_t>( self->m_values.size() ) );
		self->m_values.push_back( 0 );
	}
	if( value && PyObject_SetAttr( reinterpret_cast<PyObject*>( self ), name, value ) < 0 )
	{
		return 0;
	}
	return cppy::incref( Py_None );
}


PyObject* Atom_sizeof( Atom* self, PyObject* args )
{
	// TODO account for extra members dict size
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
	{ "get_class_member",
	  ( PyCFunction )Atom_get_class_member,
	  METH_O,
	  "get_class_member(name) get the named class member for the object or None" },
	{ "get_extra_member",
	  ( PyCFunction )Atom_get_extra_member,
	  METH_O,
	  "get_extra_member(name) get the named extra member for the object or None" },
	{ "get_members",
	  ( PyCFunction )Atom_get_members,
	  METH_NOARGS,
	  "get_members() get all of the members for the object as a dict" },
	{ "get_class_members",
	  ( PyCFunction )Atom_get_class_members,
	  METH_NOARGS,
	  "get_class_members() get the class members for the object as a dict" },
	{ "get_extra_members",
	  ( PyCFunction )Atom_get_extra_members,
	  METH_NOARGS,
	  "get_extra_members() get the extra members for the object as a dict" },
	{ "add_extra_member",
	  ( PyCFunction )Atom_add_extra_member,
	  METH_VARARGS,
	  "add_extra_member(name, member) add an extra member to the object" },
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
	offsetof(Atom, m_weakreflist),       /* tp_weaklistoffset */
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
	class_members_registry = PyDict_New();
	if( !class_members_registry )
	{
		return false;
	}
	return PyType_Ready( &TypeObject ) == 0;
}


bool Atom::RegisterMembers( PyTypeObject* type, PyObject* members )
{
	PyObject* pyo = reinterpret_cast<PyObject*>( type );
	return PyDict_SetItem( class_members_registry, pyo, members ) == 0;
}


PyObject* Atom::LookupMembers( PyTypeObject* type )
{
	PyObject* pyo = reinterpret_cast<PyObject*>( type );
	PyObject* members = PyDict_GetItem( class_members_registry, pyo );
	if( members )
	{
		return cppy::incref( members );
	}
	return cppy::type_error( "type has no registered members" );
}

} // namespace atom
