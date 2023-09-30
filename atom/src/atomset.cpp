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

namespace atom
{

namespace
{

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
#if PY_VERSION_HEX >= 0x03090000
    // This was not needed before Python 3.9 (Python issue 35810 and 40217)
    Py_VISIT(Py_TYPE(self));
#endif
    // PySet_type is not heap allocated so it does visit the type
	return PySet_Type.tp_traverse( pyobject_cast( self ), visit, arg );
}


void AtomSet_dealloc( AtomSet* self )
{
	PyObject_GC_UnTrack( self );
	cppy::clear( &self->m_value_validator );
	delete atomset_cast( self )->pointer;
    atomset_cast( self )->pointer = 0;
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
	if( AtomSet::Update( self, value ) < 0 )
	{
		return 0;
	}
	return cppy::incref( Py_None );
}


static PyMethodDef AtomSet_methods[] = {
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


static PyType_Slot AtomSet_Type_slots[] = {
    { Py_tp_dealloc, void_cast( AtomSet_dealloc ) },         /* tp_dealloc */
    { Py_tp_traverse, void_cast( AtomSet_traverse ) },       /* tp_traverse */
    { Py_tp_clear, void_cast( AtomSet_clear ) },             /* tp_clear */
    { Py_tp_methods, void_cast( AtomSet_methods ) },         /* tp_methods */
    { Py_tp_base, void_cast( &PySet_Type ) },                /* tp_base */
    { Py_tp_new, void_cast( AtomSet_new ) },                 /* tp_new */
    { Py_nb_inplace_subtract, void_cast( AtomSet_isub ) },   /* nb_inplace_substract */
    { Py_nb_inplace_and, void_cast( AtomSet_iand ) },        /* nb_inplace_substract */
    { Py_nb_inplace_xor, void_cast( AtomSet_ixor ) },        /* nb_inplace_substract */
    { Py_nb_inplace_or, void_cast( AtomSet_ior ) },          /* nb_inplace_substract */
    { 0, 0 },
};


}  // namespace


PyTypeObject* AtomSet::TypeObject = NULL;


PyType_Spec AtomSet::TypeObject_Spec = {
	PACKAGE_TYPENAME( "atomset" ),             /* tp_name */
	sizeof( AtomSet ),                         /* tp_basicsize */
	0,                                         /* tp_itemsize */
	Py_TPFLAGS_DEFAULT
	| Py_TPFLAGS_BASETYPE
	| Py_TPFLAGS_HAVE_GC
	| Py_TPFLAGS_HAVE_VERSION_TAG,              /* tp_flags */
    AtomSet_Type_slots                          /* slots */
};


PyObject* AtomSet::New( CAtom* atom, Member* validator )
{
    cppy::ptr self( PySet_Type.tp_new( AtomSet::TypeObject, 0, 0 ) );
	if( !self )
	{
		return 0;
	}
    cppy::xincref( pyobject_cast( validator ) );
    atomset_cast( self.get() )->m_value_validator = validator;
    atomset_cast( self.get() )->pointer = new CAtomPointer( atom );
    return self.release();
}


int AtomSet::Update( AtomSet* set, PyObject* value )
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


bool AtomSet::Ready()
{
    // The reference will be handled by the module to which we will add the type
	TypeObject = pytype_cast( PyType_FromSpec( &TypeObject_Spec ) );
    if( !TypeObject )
    {
        return false;
    }
    return true;
}

}  // namespace atom
