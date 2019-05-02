/*-----------------------------------------------------------------------------
| Copyright (c) 2019, Nucleic
|
| Distributed under the terms of the BSD 3-Clause License.
|
| The full license is in the file LICENSE, distributed with this software.
|----------------------------------------------------------------------------*/
#include <cppy/cppy.h>
#include "atomset.h"
#include "packagenaming.h"


inline bool should_validate( AtomSet* set )
{
	return set->m_value_validator;
}


PyObject* validate_value( AtomSet* set, PyObject* value )
{
	CAtom* atom = set->pointer->data();
    Member* validator = set->m_value_validator;

    cppy::ptr item( cppy::incref( value ) );
    if( validator && atom )
    {
        item = validator->full_validate( atom, Py_None, item.get() );
        if( !item )
        {
            return 0;
        }
    }
    return item.release();
}


PyObject* validate_set( AtomSet* set, PyObject* value )
{
	PyObject* key;
	Py_hash_t hash;
	Py_ssize_t pos = 0;
    cppy::ptr val_set( PySet_New( 0 ) );
    cppy::ptr temp;
	while( _PySet_NextEntry( value, &pos, &key, &hash ) )
	{
        temp = validate_value( set, key );
		if( !temp )
		{
			return 0;
		}
        if( PySet_Add( val_set.get(), temp.get() ) < 0 )
        {
            return 0;
        }
	}
	return val_set.release();
}


PyObject* AtomSet_New( CAtom* atom, Member* validator )
{
    cppy::ptr self( PySet_Type.tp_new( &AtomSet_Type, 0, 0 ) );
	if( !self )
	{
		return 0;
	}
    cppy::xincref( pyobject_cast( validator ) );
    atomset_cast( self.get() )->m_value_validator = validator;
    atomset_cast( self.get() )->pointer = new CAtomPointer( atom );
    return self.release();
}


int AtomSet_Update( AtomSet* set, PyObject* value )
{
	if( !should_validate( set ) )
	{
		return _PySet_Update( pyobject_cast( set ), value );
	}
	cppy::ptr temp( cppy::incref( value ) );
	if( !PyAnySet_Check( value ) && !( temp = PySet_New( value ) ) )
	{
		return -1;
	}
    temp = validate_set( set, temp.get() );
	if( !temp )
	{
		return -1;
	}
	return _PySet_Update( pyobject_cast( set ), temp.get() );
}


PyObject* AtomSet_new( PyTypeObject* type, PyObject* args, PyObject* kwargs )
{
    cppy::ptr self( PySet_Type.tp_new( type, args, kwargs ) );
	if( !self )
	{
		return 0;
	}
    atomset_cast( self.get() )->pointer = new CAtomPointer();
    return self.release();
}


int AtomSet_clear( AtomSet* self )
{
	Py_CLEAR( self->m_value_validator );
	return PySet_Type.tp_clear( pyobject_cast( self ) );
}


int AtomSet_traverse( AtomSet* self, visitproc visit, void* arg )
{
	Py_VISIT( self->m_value_validator );
	return PySet_Type.tp_traverse( pyobject_cast( self ), visit, arg );
}


void AtomSet_dealloc( AtomSet* self )
{
	cppy::clear( &self->m_value_validator );
	PySet_Type.tp_dealloc( pyobject_cast( self ) );
}


PyObject* AtomSet_isub( AtomSet* self, PyObject* other )
{
    cppy::ptr other_ptr( cppy::incref( other ) );
	if( should_validate( self ) && PyAnySet_Check( other ) )
    {
        other_ptr = validate_set( self, other );
        if( !other_ptr )
        {
            return 0;
        }
	}
	return PySet_Type.tp_as_number->nb_inplace_subtract( pyobject_cast( self ), other_ptr.get() );
}


PyObject* AtomSet_iand( AtomSet* self, PyObject* other )
{
	cppy::ptr other_ptr( cppy::incref( other ) );
	if( should_validate( self ) && PyAnySet_Check( other ) )
    {
        other_ptr = validate_set( self, other );
        if( !other_ptr )
        {
            return 0;
        }
	}
	return PySet_Type.tp_as_number->nb_inplace_and( pyobject_cast( self ), other_ptr.get() );
}


PyObject* AtomSet_ior( AtomSet* self, PyObject* other )
{
	cppy::ptr other_ptr( cppy::incref( other ) );
	if( should_validate( self ) && PyAnySet_Check( other ) )
    {
        other_ptr = validate_set( self, other );
        if( !other_ptr )
        {
            return 0;
        }
	}
	return PySet_Type.tp_as_number->nb_inplace_or( pyobject_cast( self ), other_ptr.get() );
}


PyObject* AtomSet_ixor( AtomSet* self, PyObject* other )
{
	cppy::ptr other_ptr( cppy::incref( other ) );
	if( should_validate( self ) && PyAnySet_Check( other ) )
    {
        other_ptr = validate_set( self, other );
        if( !other_ptr )
        {
            return 0;
        }
	}
	return PySet_Type.tp_as_number->nb_inplace_xor( pyobject_cast( self ), other_ptr.get() );
}



PyObject* AtomSet_add( AtomSet* self, PyObject* value )
{
    cppy::ptr value_ptr( cppy::incref( value ) );
	if( should_validate( self ) )
    {
        value_ptr = validate_value( self, value );
        if( !value_ptr )
        {
            return 0;
        }
	}
	if( PySet_Add( pyobject_cast( self ), value_ptr.get() ) < 0 )
	{
		return 0;
	}
	return cppy::incref( Py_None );
}


PyObject* AtomSet_difference_update( AtomSet* self, PyObject* value )
{
	cppy::ptr temp( cppy::incref( value ) );
	if( !PyAnySet_Check( value ) && !( temp = PySet_New( value ) ) )
	{
		return 0;
	}
	cppy::ptr ignored( AtomSet_isub( self, temp.get() ) );
	if( !ignored )
	{
		return 0;
	}
	return cppy::incref( Py_None );
}


PyObject* AtomSet_intersection_update( AtomSet* self, PyObject* value )
{
	cppy::ptr temp( cppy::incref( value ) );
	if( !PyAnySet_Check( value ) && !( temp = PySet_New( value ) ) )
	{
		return 0;
	}
	cppy::ptr ignored( AtomSet_iand( self, temp.get() ) );
	if( !ignored )
	{
		return 0;
	}
	return cppy::incref( Py_None );
}


PyObject* AtomSet_symmetric_difference_update( AtomSet* self, PyObject* value )
{
	cppy::ptr temp( cppy::incref( value ) );
	if( !PyAnySet_Check( value ) && !( temp = PySet_New( value ) ) )
	{
		return 0;
	}
	cppy::ptr ignored( AtomSet_ixor( self, temp.get() ) );
	if( !ignored )
	{
		return 0;
	}
	return cppy::incref( Py_None );
}


PyObject* AtomSet_update( AtomSet* self, PyObject* value )
{
	if( AtomSet_Update( self, value ) < 0 )
	{
		return 0;
	}
	return cppy::incref( Py_None );
}


PyNumberMethods AtomSet_as_number = {
	0,                                  /* nb_add */
	0,                                  /* nb_subtract */
	0,                                  /* nb_multiply */
	0,                                  /* nb_remainder */
	0,                                  /* nb_divmod */
	0,                                  /* nb_power */
	0,                                  /* nb_negative */
	0,                                  /* nb_positive */
	0,                                  /* nb_absolute */
	0,                                  /* nb_bool */
	0,                                  /* nb_invert */
	0,                                  /* nb_lshift */
	0,                                  /* nb_rshift */
	0,                                  /* nb_and */
	0,                                  /* nb_xor */
	0,                                  /* nb_or */
	0,                                  /* nb_int */
	0,                                  /* nb_reserved */
	0,                                  /* nb_float */
	0,                                  /* nb_inplace_add */
	(binaryfunc)AtomSet_isub,          /* nb_inplace_subtract */
	0,                                  /* nb_inplace_multiply */
	0,                                  /* nb_inplace_remainder */
	0,                                  /* nb_inplace_power */
	0,                                  /* nb_inplace_lshift */
	0,                                  /* nb_inplace_rshift */
	(binaryfunc)AtomSet_iand,          /* nb_inplace_and */
	(binaryfunc)AtomSet_ixor,          /* nb_inplace_xor */
	(binaryfunc)AtomSet_ior            /* nb_inplace_or */
};


PyMethodDef AtomSet_methods[] = {
	{ "add",
	  ( PyCFunction )AtomSet_add,
	  METH_O,
	  "Add an element to a set." },
	{ "difference_update",
	  ( PyCFunction )AtomSet_difference_update,
	  METH_O,
	  "Update a set with the difference of itself and another." },
	{ "intersection_update",
	  ( PyCFunction )AtomSet_intersection_update,
	  METH_O,
	  "Update a set with the intersection of itself and another." },
	{ "symmetric_difference_update",
	  ( PyCFunction )AtomSet_symmetric_difference_update,
	  METH_O,
	  "Update a set with the symmetric difference of itself and another." },
	{ "update",
	  ( PyCFunction )AtomSet_update,
	  METH_O,
	  "Update a set with the union of itself and another." },
	{ 0 } // sentinel
};



PyTypeObject AtomSet_Type = {
	PyVarObject_HEAD_INIT( &PyType_Type, 0 )
	PACKAGE_TYPENAME( "atomset" ),            /* tp_name */
	sizeof( AtomSet ),                        /* tp_basicsize */
	0,                                        /* tp_itemsize */
	( destructor )AtomSet_dealloc,            /* tp_dealloc */
	( printfunc )0,                           /* tp_print */
	( getattrfunc )0,                         /* tp_getattr */
	( setattrfunc )0,                         /* tp_setattr */
	( PyAsyncMethods* )0,                     /* tp_as_async */
	( reprfunc )0,                            /* tp_repr */
	( PyNumberMethods* )&AtomSet_as_number,   /* tp_as_number */
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
	( traverseproc )AtomSet_traverse,         /* tp_traverse */
	( inquiry )AtomSet_clear,                 /* tp_clear */
	( richcmpfunc )0,                         /* tp_richcompare */
	0,                                        /* tp_weaklistoffset */
	( getiterfunc )0,                         /* tp_iter */
	( iternextfunc )0,                        /* tp_iternext */
	( struct PyMethodDef* )AtomSet_methods,   /* tp_methods */
	( struct PyMemberDef* )0,                 /* tp_members */
	0,                                        /* tp_getset */
	&PySet_Type,                              /* tp_base */
	0,                                        /* tp_dict */
	( descrgetfunc )0,                        /* tp_descr_get */
	( descrsetfunc )0,                        /* tp_descr_set */
	0,                                        /* tp_dictoffset */
	( initproc )0,                            /* tp_init */
	( allocfunc )0,                           /* tp_alloc */
	( newfunc )AtomSet_new,                   /* tp_new */
	( freefunc )0,                            /* tp_free */
	( inquiry )0,                             /* tp_is_gc */
	0,                                        /* tp_bases */
	0,                                        /* tp_mro */
	0,                                        /* tp_cache */
	0,                                        /* tp_subclasses */
	0,                                        /* tp_weaklist */
	( destructor )0                           /* tp_del */
};


int
import_atomset()
{
    if( PyType_Ready( &AtomSet_Type ) < 0 )
        return -1;
    return 0;
}
