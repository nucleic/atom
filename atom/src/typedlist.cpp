/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include "typedlist.h"
#include "errors.h"
#include "utils.h"

#include <cppy/cppy.h>


#define pyobject_cast( o ) reinterpret_cast<PyObject*>( o )
#define typedlist_cast( o ) reinterpret_cast<TypedList*>( o )


namespace atom
{

namespace
{

inline void validation_error( TypedList* list, PyObject* value )
{
	// TODO impelement me
	PyErr_SetString( Errors::ValidationError, "foo" );
}


inline bool validate( TypedList* list, PyObject* value )
{
	int ok = PyObject_IsInstance( value, list->m_value_type );
	if( ok == 1 )
	{
		return true;
	}
	if( ok == 0 )
	{
		validation_error( list, value );
	}
	return false;
}


inline PyObject* validate_sequence( TypedList* list, PyObject* value )
{
	cppy::ptr res( value, true );
	if( !PyList_Check( res.get() ) && !( res = PySequence_List( value ) ) )
	{
		return 0;
	}
	for( Py_ssize_t i = 0; i < PyList_GET_SIZE( res.get() ); ++i )
	{
		if( !validate( list, PyList_GET_ITEM( res.get(), i ) ) )
		{
			return 0;
		}
	}
	return res.release();
}


PyObject* TypedList_extend( TypedList* self, PyObject* iterable );


PyObject* TypedList_new( PyTypeObject* type, PyObject* args, PyObject* kwargs )
{
	PyObject* self = PyType_GenericNew( type, args, kwargs );
	if( !self )
	{
		return 0;
	}
	// ensure m_value_type is never null
	TypedList* list = typedlist_cast( self );
	PyObject* object = pyobject_cast( &PyBaseObject_Type );
	list->m_value_type = cppy::incref( object );
	return self;
}


int TypedList_init( TypedList* self, PyObject* args, PyObject* kwargs )
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
	if( PyList_GET_SIZE( self ) > 0 )
	{
		Py_TYPE( self )->tp_clear( pyobject_cast( self ) );
	}
	cppy::replace( &self->m_value_type, value_type );
	if( sequence )
	{
		cppy::ptr res( TypedList_extend( self, sequence ) );
		if( !res )
		{
			return -1;
		}
	}
	return 0;
}


int TypedList_clear( TypedList* self )
{
	Py_CLEAR( self->m_value_type );
	return PyList_Type.tp_clear( pyobject_cast( self ) );
}


int TypedList_traverse( TypedList* self, visitproc visit, void* arg )
{
	Py_VISIT( self->m_value_type );
	return PyList_Type.tp_traverse( pyobject_cast( self ), visit, arg );
}


void TypedList_dealloc( TypedList* self )
{
	cppy::clear( &self->m_value_type );
	PyList_Type.tp_dealloc( pyobject_cast( self ) );
}


PyObject* TypedList_get_value_type( TypedList* self, void* context )
{
	return cppy::incref( self->m_value_type );
}


PyObject* TypedList_append( TypedList* self, PyObject* value )
{
	if( !validate( self, value ) )
	{
		return 0;
	}
	if( PyList_Append( pyobject_cast( self ), value ) != 0 )
	{
		return 0;
	}
	return cppy::incref( Py_None );
}


PyObject* TypedList_extend( TypedList* self, PyObject* value )
{
	cppy::ptr list( validate_sequence( self, value ) );
	if( !list )
	{
		return 0;
	}
	return _PyList_Extend( &self->list, list.get() );
}


PyObject* TypedList_insert( TypedList* self, PyObject* args )
{
	Py_ssize_t index;
	PyObject* value;
	if( !PyArg_ParseTuple( args, "nO:insert", &index, &value ) )
	{
		return 0;
	}
	if( !validate( self, value ) )
	{
		return 0;
	}
	if( PyList_Insert( pyobject_cast( self ), index, value ) != 0 )
	{
		return 0;
	}
	return cppy::incref( Py_None );
}


int TypedList_ass_item( TypedList* self, Py_ssize_t index, PyObject* value )
{
	if( value && !validate( self, value ) )
	{
		return -1;
	}
	return PyList_Type.tp_as_sequence->sq_ass_item( pyobject_cast( self ), index, value );
}


int TypedList_ass_slice( TypedList* self, Py_ssize_t low, Py_ssize_t high, PyObject* value )
{
	cppy::ptr list( value, true );
	if( list && !( list = validate_sequence( self, list.get() ) ) )
	{
		return -1;
	}
	return PyList_Type.tp_as_sequence->sq_ass_slice( pyobject_cast( self ), low, high, list.get() );
}


PyObject* TypedList_inplace_concat( TypedList* self, PyObject* value )
{
	cppy::ptr res( TypedList_extend( self, value ) );
	if( !res )
	{
		return 0;
	}
	return cppy::incref( pyobject_cast( self ) );
}


int TypedList_ass_subscript( TypedList* self, PyObject* key, PyObject* value )
{
	cppy::ptr item( value, true );
	if( value )
	{
		if( PyIndex_Check( key ) )
		{
			if( !validate( self, value ) )
			{
				return -1;
			}
		}
		else if( PySlice_Check( key ) )
		{
			if( !( item = validate_sequence( self, value ) ) )
			{
				return -1;
			}
		}
	}
	return PyList_Type.tp_as_mapping->mp_ass_subscript( pyobject_cast( self ), key, item.get() );
}


PyGetSetDef TypedList_getset[] = {
	{ "value_type",
	  ( getter )TypedList_get_value_type, ( setter )0,
	  "the value type for the list", 0 },
	{ 0 } // sentinel
};


PyMethodDef TypedList_methods[] = {
	{ "append",
		( PyCFunction )TypedList_append,
		METH_O,
		"L.append(object) -- append object to end" },
	{ "insert",
		( PyCFunction )TypedList_insert,
		METH_VARARGS,
		"L.insert(index, object) -- insert object before index" },
	{ "extend",
	  ( PyCFunction )TypedList_extend,
	  METH_O,
	  "L.extend(iterable) -- extend list by appending elements from the iterable" },
	{ 0 }  /* sentinel */
};


PySequenceMethods TypedList_as_sequence = {
    (lenfunc)0,                                 /* sq_length */
    (binaryfunc)0,                              /* sq_concat */
    (ssizeargfunc)0,                            /* sq_repeat */
    (ssizeargfunc)0,                            /* sq_item */
    (ssizessizeargfunc)0,                       /* sq_slice */
    (ssizeobjargproc)TypedList_ass_item,        /* sq_ass_item */
    (ssizessizeobjargproc)TypedList_ass_slice,  /* sq_ass_slice */
    (objobjproc)0,                              /* sq_contains */
    (binaryfunc)TypedList_inplace_concat,       /* sq_inplace_concat */
    (ssizeargfunc)0,                            /* sq_inplace_repeat */
};


PyMappingMethods TypedList_as_mapping = {
    (lenfunc)0,                             /* mp_length */
    (binaryfunc)0,                          /* mp_subscript */
    (objobjargproc)TypedList_ass_subscript  /* mp_ass_subscript */
};

} // namespace


PyTypeObject TypedList::TypeObject = {
	PyObject_HEAD_INIT( &PyType_Type )
	0,                                      /* ob_size */
	"atom.catom.TypedList",        			    /* tp_name */
	sizeof( TypedList ),                    /* tp_basicsize */
	0,                                      /* tp_itemsize */
	(destructor)TypedList_dealloc,          /* tp_dealloc */
	(printfunc)0,                           /* tp_print */
	(getattrfunc)0,                         /* tp_getattr */
	(setattrfunc)0,                         /* tp_setattr */
	(cmpfunc)0,                             /* tp_compare */
	(reprfunc)0,                            /* tp_repr */
	(PyNumberMethods*)0,                    /* tp_as_number */
	(PySequenceMethods*)&TypedList_as_sequence, /* tp_as_sequence */
	(PyMappingMethods*)&TypedList_as_mapping,   /* tp_as_mapping */
	(hashfunc)0,                            /* tp_hash */
	(ternaryfunc)0,                         /* tp_call */
	(reprfunc)0,                            /* tp_str */
	(getattrofunc)0,                        /* tp_getattro */
	(setattrofunc)0,                        /* tp_setattro */
	(PyBufferProcs*)0,                      /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT
	| Py_TPFLAGS_BASETYPE
	| Py_TPFLAGS_HAVE_GC
	| Py_TPFLAGS_HAVE_VERSION_TAG,          /* tp_flags */
	0,                                      /* Documentation string */
	(traverseproc)TypedList_traverse,       /* tp_traverse */
	(inquiry)TypedList_clear,               /* tp_clear */
	(richcmpfunc)0,                         /* tp_richcompare */
	0,                                      /* tp_weaklistoffset */
	(getiterfunc)0,                         /* tp_iter */
	(iternextfunc)0,                        /* tp_iternext */
	(struct PyMethodDef*)TypedList_methods, /* tp_methods */
	(struct PyMemberDef*)0,                 /* tp_members */
	TypedList_getset,                       /* tp_getset */
	&PyList_Type,                           /* tp_base */
	0,                                      /* tp_dict */
	(descrgetfunc)0,                        /* tp_descr_get */
	(descrsetfunc)0,                        /* tp_descr_set */
	0,                                      /* tp_dictoffset */
	(initproc)TypedList_init,               /* tp_init */
	(allocfunc)0,						                /* tp_alloc */
	(newfunc)TypedList_new,			   				  /* tp_new */
	(freefunc)0,                            /* tp_free */
	(inquiry)0,                             /* tp_is_gc */
	0,                                      /* tp_bases */
	0,                                      /* tp_mro */
	0,                                      /* tp_cache */
	0,                                      /* tp_subclasses */
	0,                                      /* tp_weaklist */
	(destructor)0                           /* tp_del */
};


bool TypedList::Ready()
{
	return PyType_Ready( &TypeObject ) == 0;
}


PyObject* TypedList::Create( PyObject* value_type, PyObject* values )
{
 	return cppy::incref( Py_None );
}


} // namespace atom
