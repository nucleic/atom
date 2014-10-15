/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include "typedset.h"
#include "errors.h"
#include "utils.h"

#include <cppy/cppy.h>

// Note: on Python 2, subclassing set is technically broken because the
// set type bypasses tp_new when creating instances of a set subtype:
// https://mail.python.org/pipermail/python-bugs-list/2007-May/038470.html
// This is fixed in Python 3, but was never backported. Therefore, extra
// checks must be done guard against a possible null m_value_type.

#define pyobject_cast( o ) reinterpret_cast<PyObject*>( o )


namespace atom
{

namespace
{

PyObject* intersection_str;


void validation_error( TypedSet* set, PyObject* value )
{
	cppy::ptr setptr( pyobject_cast( set ), true );
	cppy::ptr method( setptr.getattr( "validation_error" ) );
	if( !method )
	{
		return;
	}
	cppy::ptr args( PyTuple_Pack( 1, value ) );
	if( !args )
	{
		return;
	}
	cppy::ptr result( method.call( args ) );
	if( !result )
	{
		return;
	}
	cppy::system_error( "typed set failed to raise validation error" );
}


inline bool should_validate( TypedSet* set )
{
	return set->m_value_type && set->m_value_type != pyobject_cast( &PyBaseObject_Type );
}


// only called if should_validate(...) == true
inline bool validate_value( TypedSet* set, PyObject* value )
{
	int ok = PyObject_IsInstance( value, set->m_value_type );
	if( ok == 1 )
	{
		return true;
	}
	if( ok == 0 )
	{
		validation_error( set, value );
	}
	return false;
}


// only called if should_validate(...) == true
inline bool validate_set( TypedSet* set, PyObject* value )
{
	PyObject* key;
#ifdef IS_PY3K
	Py_hash_t hash;
#else
	long hash;
#endif
	Py_ssize_t pos = 0;
	while( _PySet_NextEntry( value, &pos, &key, &hash ) )
	{
		if( !validate_value( set, key ) )
		{
			return false;
		}
	}
	return true;
}


int update_multi( PyObject* set, PyObject* args )
{
	Py_ssize_t count = PyTuple_GET_SIZE( args );
	if( count == 0 )
	{
		return 0;
	}
	for( Py_ssize_t i = 0; i < count; ++i )
	{
		if( _PySet_Update( set, PyTuple_GET_ITEM( args, i ) ) < 0 )
		{
			return -1;
		}
	}
	return 0;
}


int TypedSet_update_common( TypedSet* set, PyObject* args )
{
	if( !should_validate( set ) )
	{
		return update_multi( pyobject_cast( set ), args );
	}
	cppy::ptr temp( PySet_New( 0 ) );
	if( !temp )
	{
		return -1;
	}
	if( update_multi( temp.get(), args ) < 0 )
	{
		return -1;
	}
	if( !validate_set( set, temp.get() ) )
	{
		return -1;
	}
	if( _PySet_Update( pyobject_cast( set ), temp.get() ) < 0 )
	{
		return -1;
	}
	return 0;
}


int TypedSet_intersection_update_common( TypedSet* set, PyObject* args )
{
	// The C API for set does not expose an intersection update method. So
	// the simplest way to do this is to intersect a copy and then clear
	// and update the set in-place. It would be much nicer if the set API
	// was as well designed as the rest of the Python builtins...
	cppy::ptr setptr( pyobject_cast( set ), true );
	cppy::ptr method( setptr.getattr( intersection_str ) );
	if( !method )
	{
		return -1;
	}
	cppy::ptr temp( method.call( args ) );
	if( !temp )
	{
		return -1;
	}
	if( !PyAnySet_Check( temp.get() ) && !( temp = PySet_New( temp.get() ) ) )
	{
		return -1;
	}
	if( should_validate( set ) && !validate_set( set, temp.get() ) )
	{
		return -1;
	}
	if( PySet_Clear( pyobject_cast( set ) ) < 0 )
	{
		return -1;
	}
	if( _PySet_Update( pyobject_cast( set ), temp.get() ) < 0 )
	{
		return -1;
	}
	return 0;
}


PyObject* TypedSet_new( PyTypeObject* type, PyObject* args, PyObject* kwargs )
{
	PyObject* self = PySet_Type.tp_new( type, args, kwargs );
	if( !self )
	{
		return 0;
	}
	TypedSet* set = reinterpret_cast<TypedSet*>( self );
	set->m_value_type = cppy::incref( pyobject_cast( &PyBaseObject_Type ) );
	return self;
}


int TypedSet_init( TypedSet* self, PyObject* args, PyObject* kwargs )
{
	PyObject* value_type;
	PyObject* sequence = 0;
	static char *kwlist[] = { "value_type", "sequence", 0 };
	if( !PyArg_ParseTupleAndKeywords( args, kwargs, "O|O", kwlist, &value_type, &sequence ) )
	{
		return -1;
	}
	if( !utils::is_type_or_tuple_of_types( value_type ) )
	{
		cppy::type_error( value_type, "type or tuple of types" );
		return -1;
	}
	if( PySet_GET_SIZE( self ) > 0 )
	{
		PySet_Clear( pyobject_cast( self ) );
	}
	cppy::replace( &self->m_value_type, value_type );
	if( sequence )
	{
		cppy::ptr args( PyTuple_Pack( 1, sequence ) );
		if( !args )
		{
			return -1;
		}
		return TypedSet_update_common( self, args.get() );
	}
	return 0;
}


int TypedSet_clear( TypedSet* self )
{
	Py_CLEAR( self->m_value_type );
	return PySet_Type.tp_clear( pyobject_cast( self ) );
}


int TypedSet_traverse( TypedSet* self, visitproc visit, void* arg )
{
	Py_VISIT( self->m_value_type );
	return PySet_Type.tp_traverse( pyobject_cast( self ), visit, arg );
}


void TypedSet_dealloc( TypedSet* self )
{
	cppy::clear( &self->m_value_type );
	PySet_Type.tp_dealloc( pyobject_cast( self ) );
}


// Set intersection keeps the elements from the second set, which makes
// validation necessary because 1 == 1.0 according to the set. grrrr...
PyObject* TypedSet_iand( TypedSet* self, PyObject* other )
{
	cppy::ptr args( PyTuple_Pack( 1, other ) );
	if( !args )
	{
		return 0;
	}
	if( TypedSet_intersection_update_common( self, args.get() ) < 0 )
	{
		return 0;
	}
	return cppy::incref( pyobject_cast( self ) );
}


PyObject* TypedSet_ior( TypedSet* self, PyObject* other )
{
	if( should_validate( self ) && PyAnySet_Check( other ) && !validate_set( self, other ) )
	{
		return 0;
	}
	return PySet_Type.tp_as_number->nb_inplace_or( pyobject_cast( self ), other );
}


PyObject* TypedSet_ixor( TypedSet* self, PyObject* other )
{
	if( should_validate( self ) && PyAnySet_Check( other ) && !validate_set( self, other ) )
	{
		return 0;
	}
	return PySet_Type.tp_as_number->nb_inplace_xor( pyobject_cast( self ), other );
}


PyObject* TypedSet_get_value_type( TypedSet* self, void* context )
{
	return cppy::incref( self->m_value_type ? self->m_value_type : pyobject_cast( &PyBaseObject_Type ) );
}


PyObject* TypedSet_add( TypedSet* self, PyObject* value )
{
	if( should_validate( self ) && !validate_value( self, value ) )
	{
		return 0;
	}
	if( PySet_Add( pyobject_cast( self ), value ) < 0 )
	{
		return 0;
	}
	return cppy::incref( Py_None );
}


PyObject* TypedSet_update( TypedSet* self, PyObject* args )
{
	if( TypedSet_update_common( self, args ) < 0 )
	{
		return 0;
	}
	return cppy::incref( Py_None );
}


// Set intersection keeps the elements from the second set, which makes
// validation necessary because 1 == 1.0 according to the set. grrrr...
PyObject* TypedSet_intersection_update( TypedSet* self, PyObject* args )
{
	if( TypedSet_intersection_update_common( self, args ) < 0 )
	{
		return 0;
	}
	return cppy::incref( Py_None );
}


PyObject* TypedSet_symmetric_difference_update( TypedSet* self, PyObject* value )
{
	cppy::ptr temp( value, true );
	if( !PyAnySet_Check( value ) && !( temp = PySet_New( value ) ) )
	{
		return 0;
	}
	cppy::ptr ignored( TypedSet_ixor( self, temp.get() ) );
	if( !ignored )
	{
		return 0;
	}
	return cppy::incref( Py_None );
}


PyObject* TypedSet_validation_error( TypedSet* self, PyObject* value )
{
	static PyObject* tsv_message = 0;
	if( !tsv_message )
	{
		cppy::ptr mod( PyImport_ImportModule( "atom._cpphelpers" ) );
		if( !mod )
		{
			return 0;
		}
		tsv_message = mod.getattr( "typed_set_validation_message" );
		if( !tsv_message )
		{
			return 0;
		}
	}
	cppy::ptr args( PyTuple_Pack( 2, pyobject_cast( self ), value ) );
	if( !args )
	{
		return 0;
	}
	cppy::ptr msg( PyObject_Call( tsv_message, args.get(), 0 ) );
	if( !msg )
	{
		return 0;
	}
	PyErr_SetObject( Errors::ValidationError, msg.get() );
	return 0;
};


PyNumberMethods TypedSet_as_number = {
	0,                                  /* nb_add */
	0,                                  /* nb_subtract */
	0,                                  /* nb_multiply */
#ifndef IS_PY3K
	0,                                  /* nb_divide */
#endif
	0,                                  /* nb_remainder */
	0,                                  /* nb_divmod */
	0,                                  /* nb_power */
	0,                                  /* nb_negative */
	0,                                  /* nb_positive */
	0,                                  /* nb_absolute */
	0,                                  /* nb_nonzero */
	0,                                  /* nb_invert */
	0,                                  /* nb_lshift */
	0,                                  /* nb_rshift */
	0,                                  /* nb_and */
	0,                                  /* nb_xor */
	0,                                  /* nb_or */
#ifndef IS_PY3K
	0,                                  /* nb_coerce */
#endif
	0,                                  /* nb_int */
	0,                                  /* nb_long */
	0,                                  /* nb_float */
#ifndef IS_PY3K
	0,                                  /* nb_oct */
	0,                                  /* nb_hex */
#endif
	0,                                  /* nb_inplace_add */
	0,                                  /* nb_inplace_subtract */
	0,                                  /* nb_inplace_multiply */
#ifndef IS_PY3K
	0,                                  /* nb_inplace_divide */
#endif
	0,                                  /* nb_inplace_remainder */
	0,                                  /* nb_inplace_power */
	0,                                  /* nb_inplace_lshift */
	0,                                  /* nb_inplace_rshift */
	(binaryfunc)TypedSet_iand,          /* nb_inplace_and */
	(binaryfunc)TypedSet_ixor,          /* nb_inplace_xor */
	(binaryfunc)TypedSet_ior            /* nb_inplace_or */
};


PyGetSetDef TypedSet_getset[] = {
	{ "value_type",
		( getter )TypedSet_get_value_type, ( setter )0,
		"the value type for the set", 0 },
	{ 0 } // sentinel
};


PyMethodDef TypedSet_methods[] = {
	{ "add",
	  ( PyCFunction )TypedSet_add,
	  METH_O,
	  "Add an element to a set." },
	{ "update",
	  ( PyCFunction )TypedSet_update,
	  METH_VARARGS,
	  "Update a set with the union of itself and others." },
	{ "intersection_update",
	  ( PyCFunction )TypedSet_intersection_update,
	  METH_VARARGS,
	  "Update a set with the intersection of itself and another." },
	{ "symmetric_difference_update",
	  ( PyCFunction )TypedSet_symmetric_difference_update,
	  METH_O,
	  "Update a set with the symmetric difference of itself and another." },
	{ "validation_error",
		( PyCFunction )TypedSet_validation_error,
		METH_O,
		"Raise a ValidationError for the given value." },
	{ 0 } // sentinel
};

} // namespace


PyTypeObject TypedSet::TypeObject = {
	PyVarObject_HEAD_INIT( &PyType_Type, 0 )
	"atom.catom.TypedSet",                    /* tp_name */
	sizeof( TypedSet ),                       /* tp_basicsize */
	0,                                        /* tp_itemsize */
	( destructor )TypedSet_dealloc,           /* tp_dealloc */
	( printfunc )0,                           /* tp_print */
	( getattrfunc )0,                         /* tp_getattr */
	( setattrfunc )0,                         /* tp_setattr */
#ifdef IS_PY3K
	( void* )0,                               /* tp_reserved */
#else
	( cmpfunc )0,                             /* tp_compare */
#endif
	( reprfunc )0,                            /* tp_repr */
	( PyNumberMethods* )&TypedSet_as_number,   /* tp_as_number */
	( PySequenceMethods* )0,                  /* tp_as_sequence */
	( PyMappingMethods* )0,                   /* tp_as_mapping */
	( hashfunc )0,                            /* tp_hash */
	( ternaryfunc )0,                         /* tp_call */
	( reprfunc )0,                            /* tp_str */
	( getattrofunc )0,                        /* tp_getattro */
	( setattrofunc )0,                        /* tp_setattro */
	( PyBufferProcs* )0,                      /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT
	| Py_TPFLAGS_BASETYPE
	| Py_TPFLAGS_HAVE_GC
	| Py_TPFLAGS_HAVE_VERSION_TAG,            /* tp_flags */
	0,                                        /* Documentation string */
	( traverseproc )TypedSet_traverse,        /* tp_traverse */
	( inquiry )TypedSet_clear,                /* tp_clear */
	( richcmpfunc )0,                         /* tp_richcompare */
	0,                                        /* tp_weaklistoffset */
	( getiterfunc )0,                         /* tp_iter */
	( iternextfunc )0,                        /* tp_iternext */
	( struct PyMethodDef* )TypedSet_methods,  /* tp_methods */
	( struct PyMemberDef* )0,                 /* tp_members */
	TypedSet_getset,                          /* tp_getset */
	&PySet_Type,                              /* tp_base */
	0,                                        /* tp_dict */
	( descrgetfunc )0,                        /* tp_descr_get */
	( descrsetfunc )0,                        /* tp_descr_set */
	0,                                        /* tp_dictoffset */
	( initproc )TypedSet_init,                /* tp_init */
	( allocfunc )0,                           /* tp_alloc */
	( newfunc )TypedSet_new,                  /* tp_new */
	( freefunc )0,                            /* tp_free */
	( inquiry )0,                             /* tp_is_gc */
	0,                                        /* tp_bases */
	0,                                        /* tp_mro */
	0,                                        /* tp_cache */
	0,                                        /* tp_subclasses */
	0,                                        /* tp_weaklist */
	( destructor )0                           /* tp_del */
};


bool TypedSet::Ready()
{
	if( !( intersection_str = Py23Str_FromString( "intersection" ) ) )
	{
		return false;
	}
	return PyType_Ready( &TypeObject ) == 0;
}

} // namespace atom
