/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2019, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file LICENSE, distributed with this software.
|----------------------------------------------------------------------------*/
#include <cppy/cppy.h>
#include "methodwrapper.h"
#include "catom.h"
#include "catompointer.h"
#include "packagenaming.h"


namespace atom
{


namespace
{


/*-----------------------------------------------------------------------------
| MethodWrapper
|----------------------------------------------------------------------------*/

void
MethodWrapper_dealloc( MethodWrapper* self )
{
    Py_CLEAR( self->im_selfref );
    Py_CLEAR( self->im_func );
    Py_TYPE(self)->tp_free( pyobject_cast( self ) );
}


PyObject*
MethodWrapper__call__( MethodWrapper* self, PyObject* args, PyObject* kwargs )
{
    PyObject* im_self = PyWeakref_GET_OBJECT( self->im_selfref );
    if( im_self != Py_None )
    {
        cppy::ptr method( PyMethod_New( self->im_func, im_self ) );
        if( !method )
            return 0;
        return PyObject_Call( method.get(), args, kwargs );
    }
    Py_RETURN_NONE;
}


PyObject*
MethodWrapper_richcompare( MethodWrapper* self, PyObject* other, int op )
{
    if( op == Py_EQ )
    {
        if( PyMethod_Check( other ) && PyMethod_GET_SELF( other ) )
        {
            if( ( self->im_func == PyMethod_GET_FUNCTION( other ) ) &&
                ( PyWeakref_GET_OBJECT( self->im_selfref ) == PyMethod_GET_SELF( other ) ) )
                Py_RETURN_TRUE;
            Py_RETURN_FALSE;
        }
        else if( MethodWrapper::TypeCheck( other ) )
        {
            MethodWrapper* wrapper = reinterpret_cast<MethodWrapper*>( other );
            if( ( self->im_func == wrapper->im_func ) &&
                ( self->im_selfref == wrapper->im_selfref ) )
                Py_RETURN_TRUE;
            Py_RETURN_FALSE;
        }
        else
            Py_RETURN_FALSE;
    }
    Py_RETURN_NOTIMPLEMENTED;
}


int
MethodWrapper__bool__( MethodWrapper* self )
{
    if( PyWeakref_GET_OBJECT( self->im_selfref ) != Py_None )
        return 1;
    return 0;
}


static PyType_Slot MethodWrapper_Type_slots[] = {
    { Py_tp_dealloc, void_cast( MethodWrapper_dealloc ) },          /* tp_dealloc */
    { Py_tp_call, void_cast( MethodWrapper__call__ ) },             /* tp_call */
    { Py_tp_richcompare, void_cast( MethodWrapper_richcompare ) },  /* tp_richcompare */
    { Py_tp_alloc, void_cast( PyType_GenericAlloc ) },              /* tp_alloc */
    { Py_tp_free, void_cast( PyObject_Del ) },                      /* tp_free */
    { Py_nb_bool, void_cast( MethodWrapper__bool__ ) },             /* nb_bool */
    { 0, 0 },
};


}  // namespace


// Initialize static variables (otherwise the compiler eliminates them)
PyTypeObject* MethodWrapper::TypeObject = NULL;


PyType_Spec MethodWrapper::TypeObject_Spec = {
	PACKAGE_TYPENAME( "MethodWrapper" ),             /* tp_name */
	sizeof( MethodWrapper ),                         /* tp_basicsize */
	0,                                               /* tp_itemsize */
	Py_TPFLAGS_DEFAULT,                              /* tp_flags */
    MethodWrapper_Type_slots                           /* slots */
};

bool
MethodWrapper::Ready()
{
    // The reference will be handled by the module to which we will add the type
	TypeObject = pytype_cast( PyType_FromSpec( &TypeObject_Spec ) );
    if( !TypeObject )
    {
        return false;
    }
    return true;
}


/*-----------------------------------------------------------------------------
| AtomMethodWrapper
|----------------------------------------------------------------------------*/

namespace
{

void
AtomMethodWrapper_dealloc( AtomMethodWrapper* self )
{
    Py_CLEAR( self->im_func );
    // manual destructor since Python malloc'd and zero'd the struct
    self->pointer.~CAtomPointer();
    Py_TYPE(self)->tp_free( pyobject_cast( self ) );
}


PyObject*
AtomMethodWrapper__call__( AtomMethodWrapper* self, PyObject* args, PyObject* kwargs )
{
    if( self->pointer.data() )
    {
        PyObject* im_self = pyobject_cast( self->pointer.data() );
        cppy::ptr method( PyMethod_New( self->im_func, im_self ) );
        if( !method )
            return 0;
        return PyObject_Call( method.get(), args, kwargs );
    }
    Py_RETURN_NONE;
}


PyObject*
AtomMethodWrapper_richcompare( AtomMethodWrapper* self, PyObject* other, int op )
{
    if( op == Py_EQ )
    {
        if( PyMethod_Check( other ) && PyMethod_GET_SELF( other ) )
        {
            if( ( self->im_func == PyMethod_GET_FUNCTION( other ) ) &&
                ( pyobject_cast( self->pointer.data() ) == PyMethod_GET_SELF( other ) ) )
                Py_RETURN_TRUE;
            Py_RETURN_FALSE;
        }
        else if( AtomMethodWrapper::TypeCheck( other ) )
        {
            AtomMethodWrapper* wrapper = reinterpret_cast<AtomMethodWrapper*>( other );
            if( ( self->im_func == wrapper->im_func ) &&
                ( self->pointer.data() == wrapper->pointer.data() ) )
                Py_RETURN_TRUE;
            Py_RETURN_FALSE;
        }
        else
            Py_RETURN_FALSE;
    }
    Py_RETURN_NOTIMPLEMENTED;
}


int
AtomMethodWrapper__bool__( AtomMethodWrapper* self )
{
    if( self->pointer.data() )
        return 1;
    return 0;
}


static PyType_Slot AtomMethodWrapper_Type_slots[] = {
    { Py_tp_dealloc, void_cast( AtomMethodWrapper_dealloc ) },          /* tp_dealloc */
    { Py_tp_call, void_cast( AtomMethodWrapper__call__ ) },             /* tp_call */
    { Py_tp_richcompare, void_cast( AtomMethodWrapper_richcompare ) },  /* tp_richcompare */
    { Py_tp_alloc, void_cast( PyType_GenericAlloc ) },                  /* tp_alloc */
    { Py_tp_free, void_cast( PyObject_Del ) },                          /* tp_free */
    { Py_nb_bool, void_cast( AtomMethodWrapper__bool__ ) },             /* nb_bool */
    { 0, 0 },
};

}  // namespace


// Initialize static variables (otherwise the compiler eliminates them)
PyTypeObject* AtomMethodWrapper::TypeObject = NULL;


PyType_Spec AtomMethodWrapper::TypeObject_Spec = {
	PACKAGE_TYPENAME( "AtomMethodWrapper" ),             /* tp_name */
	sizeof( MethodWrapper ),                         /* tp_basicsize */
	0,                                               /* tp_itemsize */
	Py_TPFLAGS_DEFAULT,                              /* tp_flags */
    AtomMethodWrapper_Type_slots                           /* slots */
};


bool
AtomMethodWrapper::Ready()
{
    // The reference will be handled by the module to which we will add the type
	TypeObject = pytype_cast( PyType_FromSpec( &TypeObject_Spec ) );
    if( !TypeObject )
    {
        return false;
    }
    return true;
}


/*-----------------------------------------------------------------------------
| External API
|----------------------------------------------------------------------------*/
PyObject*
MethodWrapper::New( PyObject* method )
{
    if( !PyMethod_Check( method ) )
        return cppy::type_error( method, "MethodType" );
    if( !PyMethod_GET_SELF( method ) )
        return cppy::type_error( "cannot wrap unbound method" );
    cppy::ptr pywrapper;
    if( CAtom::TypeCheck( PyMethod_GET_SELF( method ) ) )
    {
        pywrapper = PyType_GenericNew( AtomMethodWrapper::TypeObject, 0, 0 );
        if( !pywrapper )
            return 0;
        AtomMethodWrapper* wrapper = reinterpret_cast<AtomMethodWrapper*>( pywrapper.get() );
        wrapper->im_func = cppy::incref( PyMethod_GET_FUNCTION( method ) );
        // placement new since Python malloc'd and zero'd the struct
        new( &wrapper->pointer ) CAtomPointer( catom_cast( PyMethod_GET_SELF( method ) ) );
    }
    else
    {
        cppy::ptr wr( PyWeakref_NewRef( PyMethod_GET_SELF( method ), 0 ) );
        if( !wr )
            return 0;
        pywrapper = PyType_GenericNew( MethodWrapper::TypeObject, 0, 0 );
        if( !pywrapper )
            return 0;
        MethodWrapper* wrapper = reinterpret_cast<MethodWrapper*>( pywrapper.get() );
        wrapper->im_func = cppy::incref( PyMethod_GET_FUNCTION( method ) );
        wrapper->im_selfref = wr.release();
    }
    return pywrapper.release();
}


}  // namespace atom
