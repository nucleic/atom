/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include <Python.h>
#include "inttypes.h"
#include "member.h"

#include "ignoredwarnings.h"


static PyObject*
Member_new( PyTypeObject* type, PyObject* args, PyObject* kwargs )
{
    PyObject* self = PyType_GenericNew( type, args, kwargs );
    if( !self )
    {
        return 0;
    }
    return self;
}


static void
Member_clear( Member* self )
{
}


static int
Member_traverse( Member* self, visitproc visit, void* arg )
{
    return 0;
}


static void
Member_dealloc( Member* self )
{
    PyObject_GC_UnTrack( self );
    Member_clear( self );
    self->ob_type->tp_free( reinterpret_cast<PyObject*>( self ) );
}


PyTypeObject Member_Type = {
    PyObject_HEAD_INIT( &PyType_Type )
    0,                                      /* ob_size */
    "atom.catom.Member",                    /* tp_name */
    sizeof( Member ),                       /* tp_basicsize */
    0,                                      /* tp_itemsize */
    (destructor)Member_dealloc,             /* tp_dealloc */
    (printfunc)0,                           /* tp_print */
    (getattrfunc)0,                         /* tp_getattr */
    (setattrfunc)0,                         /* tp_setattr */
    (cmpfunc)0,                             /* tp_compare */
    (reprfunc)0,                            /* tp_repr */
    (PyNumberMethods*)0,                    /* tp_as_number */
    (PySequenceMethods*)0,                  /* tp_as_sequence */
    (PyMappingMethods*)0,                   /* tp_as_mapping */
    (hashfunc)0,                            /* tp_hash */
    (ternaryfunc)0,                         /* tp_call */
    (reprfunc)0,                            /* tp_str */
    (getattrofunc)0,                        /* tp_getattro */
    (setattrofunc)0,                        /* tp_setattro */
    (PyBufferProcs*)0,                      /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE|Py_TPFLAGS_HAVE_GC|Py_TPFLAGS_HAVE_VERSION_TAG, /* tp_flags */
    0,                                      /* Documentation string */
    (traverseproc)Member_traverse,          /* tp_traverse */
    (inquiry)Member_clear,                  /* tp_clear */
    (richcmpfunc)0,                         /* tp_richcompare */
    0,                                      /* tp_weaklistoffset */
    (getiterfunc)0,                         /* tp_iter */
    (iternextfunc)0,                        /* tp_iternext */
    (struct PyMethodDef*)0,                 /* tp_methods */
    (struct PyMemberDef*)0,                 /* tp_members */
    0,                                      /* tp_getset */
    0,                                      /* tp_base */
    0,                                      /* tp_dict */
    (descrgetfunc)0,                        /* tp_descr_get */
    (descrsetfunc)0,                        /* tp_descr_set */
    0,                                      /* tp_dictoffset */
    (initproc)0,                            /* tp_init */
    (allocfunc)PyType_GenericAlloc,         /* tp_alloc */
    (newfunc)Member_new,                    /* tp_new */
    (freefunc)PyObject_GC_Del,              /* tp_free */
    (inquiry)0,                             /* tp_is_gc */
    0,                                      /* tp_bases */
    0,                                      /* tp_mro */
    0,                                      /* tp_cache */
    0,                                      /* tp_subclasses */
    0,                                      /* tp_weaklist */
    (destructor)0                           /* tp_del */
};


int
import_member()
{
    if( PyType_Ready( &Member_Type ) < 0 )
    {
        return -1;
    }
    return 0;
}
