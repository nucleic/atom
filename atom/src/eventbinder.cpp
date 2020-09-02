/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2019, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file LICENSE, distributed with this software.
|----------------------------------------------------------------------------*/
#include <cppy/cppy.h>
#include "eventbinder.h"
#include "packagenaming.h"


namespace atom
{


namespace
{


#define FREELIST_MAX 128
static int numfree = 0;
static EventBinder* freelist[ FREELIST_MAX ];


void
EventBinder_clear( EventBinder* self )
{
    Py_CLEAR( self->member );
    Py_CLEAR( self->atom );
}


int
EventBinder_traverse( EventBinder* self, visitproc visit, void* arg )
{
    Py_VISIT( self->member );
    Py_VISIT( self->atom );
#if PY_VERSION_HEX >= 0x03090000
    // This was not needed before Python 3.9 (Python issue 35810 and 40217)
    Py_VISIT(Py_TYPE(self));
#endif
    return 0;
}


void
EventBinder_dealloc( EventBinder* self )
{
    PyObject_GC_UnTrack( self );
    EventBinder_clear( self );
    if( numfree < FREELIST_MAX )
        freelist[ numfree++ ] = self;
    else
        Py_TYPE(self)->tp_free( pyobject_cast( self ) );
}


PyObject*
EventBinder_richcompare( EventBinder* self, PyObject* other, int op )
{
    if( op == Py_EQ )
    {
        if( EventBinder::TypeCheck( other ) )
        {
            EventBinder* binder = reinterpret_cast<EventBinder*>( other );
            if( self->member == binder->member && self->atom == binder->atom )
                Py_RETURN_TRUE;
            Py_RETURN_FALSE;
        }
        else
            Py_RETURN_FALSE;
    }
    Py_RETURN_NOTIMPLEMENTED;
}


PyObject*
EventBinder_bind( EventBinder* self, PyObject* callback )
{
    if( !self->atom->observe( self->member->name, callback ) )
        return 0;
    Py_RETURN_NONE;
}


PyObject*
EventBinder_unbind( EventBinder* self, PyObject* callback )
{
    if( !self->atom->unobserve( self->member->name, callback ) )
        return 0;
    Py_RETURN_NONE;
}


PyObject*
EventBinder__call__( EventBinder* self, PyObject* args, PyObject* kwargs )
{
    if( kwargs && ( PyDict_Size( kwargs ) > 0 ) )
        return cppy::type_error( "An event cannot be triggered with keyword arguments" );
    Py_ssize_t size = PyTuple_GET_SIZE( args );
    if( size > 1 )
        return cppy::type_error( "An event can be triggered with at most 1 argument" );
    PyObject* value = size == 0 ? Py_None : PyTuple_GET_ITEM( args, 0 );
    if( self->member->setattr( self->atom, value ) < 0 )
        return 0;
    Py_RETURN_NONE;
}


static PyMethodDef
EventBinder_methods[] = {
    { "bind", ( PyCFunction )EventBinder_bind, METH_O,
      "Bind a handler to the event. This is equivalent to observing the event." },
    { "unbind", ( PyCFunction )EventBinder_unbind, METH_O,
      "Unbind a handler from the event. This is equivalent to unobserving the event." },
    { 0 } // sentinel
};

static PyType_Slot EventBinder_Type_slots[] = {
    { Py_tp_dealloc, void_cast( EventBinder_dealloc ) },          /* tp_dealloc */
    { Py_tp_traverse, void_cast( EventBinder_traverse ) },        /* tp_traverse */
    { Py_tp_clear, void_cast( EventBinder_clear ) },              /* tp_clear */
    { Py_tp_methods, void_cast( EventBinder_methods ) },          /* tp_methods */
    { Py_tp_call, void_cast( EventBinder__call__ ) },             /* tp_call */
    { Py_tp_richcompare, void_cast( EventBinder_richcompare ) },  /* tp_richcompare */
    { Py_tp_alloc, void_cast( PyType_GenericAlloc ) },            /* tp_alloc */
    { Py_tp_free, void_cast( PyObject_GC_Del ) },                 /* tp_free */
    { 0, 0 },
};


}  // namespace


// Initialize static variables (otherwise the compiler eliminates them)
PyTypeObject* EventBinder::TypeObject = NULL;


PyType_Spec EventBinder::TypeObject_Spec = {
	PACKAGE_TYPENAME( "EventBinder" ),             /* tp_name */
	sizeof( EventBinder ),                         /* tp_basicsize */
	0,                                             /* tp_itemsize */
	Py_TPFLAGS_DEFAULT|
    Py_TPFLAGS_HAVE_GC,                            /* tp_flags */
    EventBinder_Type_slots                         /* slots */
};


bool
EventBinder::Ready()
{
    // The reference will be handled by the module to which we will add the type
	TypeObject = pytype_cast( PyType_FromSpec( &TypeObject_Spec ) );
    if( !TypeObject )
    {
        return false;
    }
    return true;
}


PyObject*
EventBinder::New( Member* member, CAtom* atom )
{
    PyObject* pybinder;
    if( numfree > 0 )
    {
        pybinder = pyobject_cast( freelist[ --numfree ] );
        _Py_NewReference( pybinder );
    }
    else
    {
        pybinder = PyType_GenericAlloc( TypeObject, 0 );
        if( !pybinder )
            return 0;
    }
    Py_INCREF( pyobject_cast( atom ) );
    Py_INCREF( pyobject_cast( member ) );
    EventBinder* binder = reinterpret_cast<EventBinder*>( pybinder );
    binder->member = member;
    binder->atom = atom;
    return pybinder;
}


} // namespace atom
