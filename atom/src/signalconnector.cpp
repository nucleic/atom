/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2019, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file LICENSE, distributed with this software.
|----------------------------------------------------------------------------*/
#include "signalconnector.h"
#include "packagenaming.h"


namespace atom
{


namespace
{


#define FREELIST_MAX 128
static int numfree = 0;
static SignalConnector* freelist[ FREELIST_MAX ];


void
SignalConnector_clear( SignalConnector* self )
{
    Py_CLEAR( self->member );
    Py_CLEAR( self->atom );
}


int
SignalConnector_traverse( SignalConnector* self, visitproc visit, void* arg )
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
SignalConnector_dealloc( SignalConnector* self )
{
    PyObject_GC_UnTrack( self );
    SignalConnector_clear( self );
    if( numfree < FREELIST_MAX )
        freelist[ numfree++ ] = self;
    else
        Py_TYPE(self)->tp_free( pyobject_cast( self ) );
}


PyObject*
SignalConnector_richcompare( SignalConnector* self, PyObject* other, int op )
{
    if( op == Py_EQ )
    {
        if( SignalConnector::TypeCheck( other ) )
        {
            SignalConnector* connector = reinterpret_cast<SignalConnector*>( other );
            if( self->member == connector->member && self->atom == connector->atom )
                Py_RETURN_TRUE;
            Py_RETURN_FALSE;
        }
        else
            Py_RETURN_FALSE;
    }
    Py_RETURN_NOTIMPLEMENTED;
}


PyObject*
SignalConnector__call__( SignalConnector* self, PyObject* args, PyObject* kwargs )
{
    // XXX validate the Signal args and kwargs?
    if( self->atom->get_notifications_enabled() )
    {
        if( self->member->has_observers() )
        {
            if( !self->member->notify( self->atom, args, kwargs ) )
                return 0;
        }
        if( self->atom->has_observers( self->member->name ) )
        {
            if( !self->atom->notify( self->member->name, args, kwargs ) )
                return 0;
        }
    }
    Py_RETURN_NONE;
}


PyObject*
SignalConnector_emit( SignalConnector* self, PyObject* args, PyObject* kwargs )
{
    return SignalConnector__call__( self, args, kwargs );
}


PyObject*
SignalConnector_connect( SignalConnector* self, PyObject* callback )
{
    if( !self->atom->observe( self->member->name, callback ) )
        return 0;
    Py_RETURN_NONE;
}


PyObject*
SignalConnector_disconnect( SignalConnector* self, PyObject* callback )
{
    if( !self->atom->unobserve( self->member->name, callback ) )
        return 0;
    Py_RETURN_NONE;
}


static PyMethodDef
SignalConnector_methods[] = {
    { "emit", ( PyCFunction )SignalConnector_emit, METH_VARARGS | METH_KEYWORDS,
      "Emit the signal with positional and keywords arguments. This is equivalent to calling the signal." },
    { "connect", ( PyCFunction )SignalConnector_connect, METH_O,
      "Connect a callback to the signal. This is equivalent to observing the signal." },
    { "disconnect", ( PyCFunction )SignalConnector_disconnect, METH_O,
      "Disconnect a callback from the signal. This is equivalent to unobserving the signal." },
    { 0 } // sentinel
};


static PyType_Slot SignalConnector_Type_slots[] = {
    { Py_tp_dealloc, void_cast( SignalConnector_dealloc ) },          /* tp_dealloc */
    { Py_tp_traverse, void_cast( SignalConnector_traverse ) },        /* tp_traverse */
    { Py_tp_clear, void_cast( SignalConnector_clear ) },              /* tp_clear */
    { Py_tp_methods, void_cast( SignalConnector_methods ) },          /* tp_methods */
    { Py_tp_call, void_cast( SignalConnector__call__ ) },             /* tp_call */
    { Py_tp_richcompare, void_cast( SignalConnector_richcompare ) },  /* tp_richcompare */
    { Py_tp_alloc, void_cast( PyType_GenericAlloc ) },                /* tp_alloc */
    { Py_tp_free, void_cast( PyObject_GC_Del ) },                     /* tp_free */
    { 0, 0 },
};

}  // namespace


// Initialize static variables (otherwise the compiler eliminates them)
PyTypeObject* SignalConnector::TypeObject = NULL;


PyType_Spec SignalConnector::TypeObject_Spec = {
	PACKAGE_TYPENAME( "SignalConnector" ),             /* tp_name */
	sizeof( SignalConnector ),                         /* tp_basicsize */
	0,                                             /* tp_itemsize */
	Py_TPFLAGS_DEFAULT|
    Py_TPFLAGS_HAVE_GC,                            /* tp_flags */
    SignalConnector_Type_slots                         /* slots */
};


bool
SignalConnector::Ready()
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
SignalConnector::New( atom::Member* member, atom::CAtom* atom )
{
    PyObject* pyconnector;
    if( numfree > 0 )
    {
        pyconnector = pyobject_cast( freelist[ --numfree ] );
        _Py_NewReference( pyconnector );
    }
    else
    {
        pyconnector = PyType_GenericAlloc( SignalConnector::TypeObject, 0 );
        if( !pyconnector )
            return 0;
    }
    Py_INCREF( pyobject_cast( atom ) );
    Py_INCREF( pyobject_cast( member ) );
    SignalConnector* connector = reinterpret_cast<SignalConnector*>( pyconnector );
    connector->member = member;
    connector->atom = atom;
    return pyconnector;
}


}  // namespace atom
