/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include "typeddict.h"
#include "errors.h"
#include "utils.h"

#include <cppy/cppy.h>


#define pyobject_cast( o ) reinterpret_cast<PyObject*>( o )


namespace atom
{

namespace
{

void validation_error( TypedDict* dict, PyObject* key, PyObject* value )
{
	cppy::ptr dictptr( pyobject_cast( dict ), true );
	cppy::ptr method( dictptr.getattr( "validation_error" ) );
	if( !method )
	{
		return;
	}
	cppy::ptr args( PyTuple_Pack( 2, key, value ) );
	if( !args )
	{
		return;
	}
	cppy::ptr result( method.call( args ) );
	if( !result )
	{
		return;
	}
	cppy::system_error( "typed dict failed to raise validation error" );
}


inline bool validate_item( TypedDict* dict, PyObject* key, PyObject* value )
{
	int key_ok = PyObject_IsInstance( key, dict->m_key_type );
	if( key_ok == 0 )
	{
		validation_error( dict, key, value );
		return false;
	}
	if( key_ok == -1 )
	{
		return false;
	}
	int val_ok = PyObject_IsInstance( value, dict->m_value_type );
	if( val_ok == 0 )
	{
		validation_error( dict, key, value );
		return false;
	}
	if( val_ok == -1 )
	{
		return false;
	}
	return true;
}


PyObject* merge_items( PyObject* item, PyObject* kwargs )
{
	cppy::ptr dict( PyDict_New() );
	if( !dict )
	{
		return 0;
	}
	int ok = 0;
	if( item )
	{
		if( PyObject_HasAttrString( item, "keys" ) )
		{
			ok = PyDict_Merge( dict.get(), item, 1 );
		}
		else
		{
			ok = PyDict_MergeFromSeq2( dict.get(), item, 1 );
		}
	}
	if( ok == 0 && kwargs )
	{
		ok = PyDict_Merge( dict.get(), kwargs, 1 );
	}
	return ok == 0 ? dict.release() : 0;
}


int TypedDict_update_common( TypedDict* dict, PyObject* args, PyObject* kwargs )
{
	PyObject* item = 0;
	if( !PyArg_UnpackTuple( args, "update", 0, 1, &item ) )
	{
		return -1;
	}
	cppy::ptr merged( merge_items( item, kwargs ) );
	if( !merged )
	{
		return -1;
	}
	PyObject* key;
	PyObject* value;
	Py_ssize_t index = 0;
	while( PyDict_Next( merged.get(), &index, &key, &value ) )
	{
		if( !validate_item( dict, key, value ) )
		{
			return -1;
		}
	}
	return PyDict_Update( pyobject_cast( dict ), merged.get() );
}


PyObject* TypedDict_new( PyTypeObject* type, PyObject* args, PyObject* kwargs )
{
	PyObject* self = PyDict_Type.tp_new( type, args, kwargs );
	if( !self )
	{
		return 0;
	}
	TypedDict* dict = reinterpret_cast<TypedDict*>( self );
	dict->m_key_type = cppy::incref( pyobject_cast( &PyBaseObject_Type ) );
	dict->m_value_type = cppy::incref( pyobject_cast( &PyBaseObject_Type ) );
	return self;
}


int TypedDict_init( TypedDict* self, PyObject* args, PyObject* kwargs )
{
	if( PyTuple_GET_SIZE( args ) < 2 )
	{
		cppy::type_error( "__init__ requires at least 2 arguments" );
		return -1;
	}
	PyObject* key_type = PyTuple_GET_ITEM( args, 0 );
	PyObject* value_type = PyTuple_GET_ITEM( args, 1 );
	if( !utils::is_type_or_tuple_of_types( key_type ) )
	{
		cppy::type_error( key_type, "type or tuple of types" );
		return -1;
	}
	if( !utils::is_type_or_tuple_of_types( value_type ) )
	{
		cppy::type_error( value_type, "type or tuple of types" );
		return -1;
	}
	cppy::ptr rest( PyTuple_GetSlice( args, 2, PyTuple_GET_SIZE( args ) ) );
	if( !rest )
	{
		return -1;
	}
	if( PyDict_Size( pyobject_cast( self ) ) > 0 )
	{
		PyDict_Clear( pyobject_cast( self ) );
	}
	cppy::replace( &self->m_key_type, key_type );
	cppy::replace( &self->m_value_type, value_type );
	return TypedDict_update_common( self, rest.get(), kwargs );
}


int TypedDict_clear( TypedDict* self )
{
	Py_CLEAR( self->m_key_type );
	Py_CLEAR( self->m_value_type );
	return PyDict_Type.tp_clear( pyobject_cast( self ) );
}


int TypedDict_traverse( TypedDict* self, visitproc visit, void* arg )
{
	Py_VISIT( self->m_key_type );
	Py_VISIT( self->m_value_type );
	return PyDict_Type.tp_traverse( pyobject_cast( self ), visit, arg );
}


void TypedDict_dealloc( TypedDict* self )
{
	cppy::clear( &self->m_key_type );
	cppy::clear( &self->m_value_type );
	PyDict_Type.tp_dealloc( pyobject_cast( self ) );
}


int TypedDict_ass_subscript( TypedDict* self, PyObject* key, PyObject* value )
{
	if( value && !validate_item( self, key, value ) )
	{
		return -1;
	}
	return PyDict_Type.tp_as_mapping->mp_ass_subscript( pyobject_cast( self ), key, value );
}


PyObject* TypedDict_get_key_type( TypedDict* self, void* context )
{
	return cppy::incref( self->m_key_type );
}


PyObject* TypedDict_get_value_type( TypedDict* self, void* context )
{
	return cppy::incref( self->m_value_type );
}


PyObject* TypedDict_setdefault( TypedDict* self, PyObject* args )
{
	PyObject* key;
	PyObject* dfv = Py_None;
	if( !PyArg_UnpackTuple( args, "setdefault", 1, 2, &key, &dfv ) )
	{
		return 0;
	}
	PyObject* value = PyDict_GetItem( pyobject_cast( self ), key );
	if( value )
	{
		return cppy::incref( value );
	}
	if( TypedDict_ass_subscript( self, key, dfv ) < 0 )
	{
		return 0;
	}
	return cppy::incref( dfv );
}


PyObject* TypedDict_update( TypedDict* self, PyObject* args, PyObject* kwargs )
{
	if( TypedDict_update_common( self, args, kwargs ) < 0 )
	{
		return 0;
	}
	return cppy::incref( Py_None );
}


PyObject* TypedDict_validation_error( TypedDict* self, PyObject* args )
{
	static PyObject* tdv_message = 0;
	if( !tdv_message )
	{
		cppy::ptr mod( PyImport_ImportModule( "atom._cpphelpers" ) );
		if( !mod )
		{
			return 0;
		}
		tdv_message = mod.getattr( "typed_dict_validation_message" );
		if( !tdv_message )
		{
			return 0;
		}
	}
	cppy::ptr callargs( PyTuple_Pack( 2, pyobject_cast( self ), args ) );
	if( !callargs )
	{
		return 0;
	}
	cppy::ptr msg( PyObject_Call( tdv_message, callargs.get(), 0 ) );
	if( !msg )
	{
		return 0;
	}
	PyErr_SetObject( Errors::ValidationError, msg.get() );
	return 0;
};


PyGetSetDef TypedDict_getset[] = {
	{ "key_type",
		( getter )TypedDict_get_key_type, ( setter )0,
		"the key type for the dict", 0 },
	{ "value_type",
		( getter )TypedDict_get_value_type, ( setter )0,
		"the value type for the dict", 0 },
	{ 0 } // sentinel
};


PyMethodDef TypedDict_methods[] = {
	{ "setdefault",
		( PyCFunction )TypedDict_setdefault,
		METH_VARARGS,
		"D.setdefault(k[,d]) -> D.get(k,d), also set D[k]=d if k not in D" },
	{ "update",
		( PyCFunction )TypedDict_update,
		METH_VARARGS | METH_KEYWORDS,
		"D.update([E, ]**F) -> None. Update D from dict/iterable E and F" },
	{ "validation_error",
		( PyCFunction )TypedDict_validation_error,
		METH_VARARGS,
		"D.validation_error(key, value) raise a ValidationError for the given item" },
	{ 0 } // sentinel
};


PyMappingMethods TypedDict_as_mapping = {
	( lenfunc )0,                             /* mp_length */
	( binaryfunc )0,                          /* mp_subscript */
	( objobjargproc )TypedDict_ass_subscript  /* mp_ass_subscript */
};

} // namespace


PyTypeObject TypedDict::TypeObject = {
	PyVarObject_HEAD_INIT( &PyType_Type, 0 )
	"atom.catom.TypedDict",                     /* tp_name */
	sizeof( TypedDict ),                        /* tp_basicsize */
	0,                                          /* tp_itemsize */
	( destructor )TypedDict_dealloc,            /* tp_dealloc */
	( printfunc )0,                             /* tp_print */
	( getattrfunc )0,                           /* tp_getattr */
	( setattrfunc )0,                           /* tp_setattr */
#ifdef IS_PY3K
	( void* )0,                                 /* tp_reserved */
#else
	( cmpfunc )0,                               /* tp_compare */
#endif
	( reprfunc )0,                              /* tp_repr */
	( PyNumberMethods* )0,                      /* tp_as_number */
	( PySequenceMethods* )0,                    /* tp_as_sequence */
	( PyMappingMethods* )&TypedDict_as_mapping, /* tp_as_mapping */
	( hashfunc )0,                              /* tp_hash */
	( ternaryfunc )0,                           /* tp_call */
	( reprfunc )0,                              /* tp_str */
	( getattrofunc )0,                          /* tp_getattro */
	( setattrofunc )0,                          /* tp_setattro */
	( PyBufferProcs* )0,                        /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT
	| Py_TPFLAGS_BASETYPE
	| Py_TPFLAGS_HAVE_GC
	| Py_TPFLAGS_HAVE_VERSION_TAG,              /* tp_flags */
	0,                                          /* Documentation string */
	( traverseproc )TypedDict_traverse,         /* tp_traverse */
	( inquiry )TypedDict_clear,                 /* tp_clear */
	( richcmpfunc )0,                           /* tp_richcompare */
	0,                                          /* tp_weaklistoffset */
	( getiterfunc )0,                           /* tp_iter */
	( iternextfunc )0,                          /* tp_iternext */
	( struct PyMethodDef* )TypedDict_methods,   /* tp_methods */
	( struct PyMemberDef* )0,                   /* tp_members */
	TypedDict_getset,                           /* tp_getset */
	&PyDict_Type,                               /* tp_base */
	0,                                          /* tp_dict */
	( descrgetfunc )0,                          /* tp_descr_get */
	( descrsetfunc )0,                          /* tp_descr_set */
	0,                                          /* tp_dictoffset */
	( initproc )TypedDict_init,                 /* tp_init */
	( allocfunc )0,                             /* tp_alloc */
	( newfunc )TypedDict_new,                   /* tp_new */
	( freefunc )0,                              /* tp_free */
	( inquiry )0,                               /* tp_is_gc */
	0,                                          /* tp_bases */
	0,                                          /* tp_mro */
	0,                                          /* tp_cache */
	0,                                          /* tp_subclasses */
	0,                                          /* tp_weaklist */
	( destructor )0                             /* tp_del */
};


bool TypedDict::Ready()
{
	return PyType_Ready( &TypeObject ) == 0;
}

} // namespace atom
