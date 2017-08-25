/*-----------------------------------------------------------------------------
| Copyright (c) 2013, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include "signalconnector.h"


using namespace PythonHelpers;


typedef struct {
    PyObject_HEAD
    Member* member;
    CAtom* atom;
} SignalConnector;


#define FREELIST_MAX 128
static int numfree = 0;
static SignalConnector* freelist[ FREELIST_MAX ];


static int
SignalConnector_Check( PyObject* object );


static void
SignalConnector_clear( SignalConnector* self )
{
    Py_CLEAR( self->member );
    Py_CLEAR( self->atom );
}


static int
SignalConnector_traverse( SignalConnector* self, visitproc visit, void* arg )
{
    Py_VISIT( self->member );
    Py_VISIT( self->atom );
    return 0;
}


static void
SignalConnector_dealloc( SignalConnector* self )
{
    PyObject_GC_UnTrack( self );
    SignalConnector_clear( self );
    if( numfree < FREELIST_MAX )
        freelist[ numfree++ ] = self;
    else
        Py_TYPE(self)->tp_free( pyobject_cast( self ) );
}


static PyObject*
SignalConnector_richcompare( SignalConnector* self, PyObject* other, int op )
{
    if( op == Py_EQ )
    {
        if( SignalConnector_Check( other ) )
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


static PyObject*
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


static PyObject*
SignalConnector_emit( SignalConnector* self, PyObject* args, PyObject* kwargs )
{
    return SignalConnector__call__( self, args, kwargs );
}


static PyObject*
SignalConnector_connect( SignalConnector* self, PyObject* callback )
{
    if( !self->atom->observe( self->member->name, callback ) )
        return 0;
    Py_RETURN_NONE;
}


static PyObject*
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


PyTypeObject SignalConnector_Type = {
    PyVarObject_HEAD_INIT( NULL, 0 )
    "SignalConnector",                      /* tp_name */
    sizeof( SignalConnector ),              /* tp_basicsize */
    0,                                      /* tp_itemsize */
    (destructor)SignalConnector_dealloc,    /* tp_dealloc */
    (printfunc)0,                           /* tp_print */
    (getattrfunc)0,                         /* tp_getattr */
    (setattrfunc)0,                         /* tp_setattr */
#if PY_MAJOR_VERSION >= 3
#if PY_MINOR_VERSION > 4
    ( PyAsyncMethods* )0,                  /* tp_as_async */
#else
    ( void* ) 0,                           /* tp_reserved */
#endif
#else
    ( cmpfunc )0,                          /* tp_compare */
#endif
    (reprfunc)0,                            /* tp_repr */
    (PyNumberMethods*)0,                    /* tp_as_number */
    (PySequenceMethods*)0,                  /* tp_as_sequence */
    (PyMappingMethods*)0,                   /* tp_as_mapping */
    (hashfunc)0,                            /* tp_hash */
    (ternaryfunc)SignalConnector__call__,   /* tp_call */
    (reprfunc)0,                            /* tp_str */
    (getattrofunc)0,                        /* tp_getattro */
    (setattrofunc)0,                        /* tp_setattro */
    (PyBufferProcs*)0,                      /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT|Py_TPFLAGS_HAVE_GC,  /* tp_flags */
    0,                                      /* Documentation string */
    (traverseproc)SignalConnector_traverse, /* tp_traverse */
    (inquiry)SignalConnector_clear,         /* tp_clear */
    (richcmpfunc)SignalConnector_richcompare, /* tp_richcompare */
    0,                                      /* tp_weaklistoffset */
    (getiterfunc)0,                         /* tp_iter */
    (iternextfunc)0,                        /* tp_iternext */
    (struct PyMethodDef*)SignalConnector_methods, /* tp_methods */
    (struct PyMemberDef*)0,                 /* tp_members */
    0,                                      /* tp_getset */
    0,                                      /* tp_base */
    0,                                      /* tp_dict */
    (descrgetfunc)0,                        /* tp_descr_get */
    (descrsetfunc)0,                        /* tp_descr_set */
    0,                                      /* tp_dictoffset */
    (initproc)0,                            /* tp_init */
    (allocfunc)PyType_GenericAlloc,         /* tp_alloc */
    (newfunc)0,                             /* tp_new */
    (freefunc)PyObject_GC_Del,              /* tp_free */
    (inquiry)0,                             /* tp_is_gc */
    0,                                      /* tp_bases */
    0,                                      /* tp_mro */
    0,                                      /* tp_cache */
    0,                                      /* tp_subclasses */
    0,                                      /* tp_weaklist */
    (destructor)0                           /* tp_del */
};


static int
SignalConnector_Check( PyObject* object )
{
    return PyObject_TypeCheck( object, &SignalConnector_Type );
}


PyObject*
SignalConnector_New( Member* member, CAtom* atom )
{
    PyObject* pyconnector;
    if( numfree > 0 )
    {
        pyconnector = pyobject_cast( freelist[ --numfree ] );
        _Py_NewReference( pyconnector );
    }
    else
    {
        pyconnector = PyType_GenericAlloc( &SignalConnector_Type, 0 );
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


int
import_signalconnector( void )
{
    if( PyType_Ready( &SignalConnector_Type ) < 0 )
        return -1;
    return 0;
}
