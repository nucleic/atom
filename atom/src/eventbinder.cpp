/*-----------------------------------------------------------------------------
| Copyright (c) 2013, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include "eventbinder.h"
#include "py23compat.h"


using namespace PythonHelpers;


typedef struct {
    PyObject_HEAD
    Member* member;
    CAtom* atom;
} EventBinder;


#define FREELIST_MAX 128
static int numfree = 0;
static EventBinder* freelist[ FREELIST_MAX ];


static int
EventBinder_Check( PyObject* object );


static void
EventBinder_clear( EventBinder* self )
{
    Py_CLEAR( self->member );
    Py_CLEAR( self->atom );
}


static int
EventBinder_traverse( EventBinder* self, visitproc visit, void* arg )
{
    Py_VISIT( self->member );
    Py_VISIT( self->atom );
    return 0;
}


static void
EventBinder_dealloc( EventBinder* self )
{
    PyObject_GC_UnTrack( self );
    EventBinder_clear( self );
    if( numfree < FREELIST_MAX )
        freelist[ numfree++ ] = self;
    else
        Py_TYPE(self)->tp_free( pyobject_cast( self ) );
}


static PyObject*
EventBinder_richcompare( EventBinder* self, PyObject* other, int op )
{
    if( op == Py_EQ )
    {
        if( EventBinder_Check( other ) )
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


static PyObject*
EventBinder_bind( EventBinder* self, PyObject* callback )
{
    if( !self->atom->observe( self->member->name, callback ) )
        return 0;
    Py_RETURN_NONE;
}


static PyObject*
EventBinder_unbind( EventBinder* self, PyObject* callback )
{
    if( !self->atom->unobserve( self->member->name, callback ) )
        return 0;
    Py_RETURN_NONE;
}


static PyObject*
EventBinder__call__( EventBinder* self, PyObject* args, PyObject* kwargs )
{
    if( kwargs && ( PyDict_Size( kwargs ) > 0 ) )
        return py_type_fail( "An event cannot be triggered with keyword arguments" );
    Py_ssize_t size = PyTuple_GET_SIZE( args );
    if( size > 1 )
        return py_type_fail( "An event can be triggered with at most 1 argument" );
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


PyTypeObject EventBinder_Type = {
    PyVarObject_HEAD_INIT( NULL, 0 )
    "EventBinder",                          /* tp_name */
    sizeof( EventBinder ),                  /* tp_basicsize */
    0,                                      /* tp_itemsize */
    (destructor)EventBinder_dealloc,        /* tp_dealloc */
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
    (ternaryfunc)EventBinder__call__,       /* tp_call */
    (reprfunc)0,                            /* tp_str */
    (getattrofunc)0,                        /* tp_getattro */
    (setattrofunc)0,                        /* tp_setattro */
    (PyBufferProcs*)0,                      /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT|Py_TPFLAGS_HAVE_GC,  /* tp_flags */
    0,                                      /* Documentation string */
    (traverseproc)EventBinder_traverse,     /* tp_traverse */
    (inquiry)EventBinder_clear,             /* tp_clear */
    (richcmpfunc)EventBinder_richcompare,   /* tp_richcompare */
    0,                                      /* tp_weaklistoffset */
    (getiterfunc)0,                         /* tp_iter */
    (iternextfunc)0,                        /* tp_iternext */
    (struct PyMethodDef*)EventBinder_methods, /* tp_methods */
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
EventBinder_Check( PyObject* object )
{
    return PyObject_TypeCheck( object, &EventBinder_Type );
}


PyObject*
EventBinder_New( Member* member, CAtom* atom )
{
    PyObject* pybinder;
    if( numfree > 0 )
    {
        pybinder = pyobject_cast( freelist[ --numfree ] );
        _Py_NewReference( pybinder );
    }
    else
    {
        pybinder = PyType_GenericAlloc( &EventBinder_Type, 0 );
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


int
import_eventbinder( void )
{
    if( PyType_Ready( &EventBinder_Type ) < 0 )
        return -1;
    return 0;
}
