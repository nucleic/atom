/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include <Python.h>
#include "pythonhelpers.h"
#include "member.h"

#include "ignoredwarnings.h"


using namespace PythonHelpers;


static int Member_init( PyObject* self, PyObject* args, PyObject* kwargs )
{
    if( PyTuple_GET_SIZE( args ) > 0 )
    {
        py_type_fail( "__init__() takes no positional arguments" );
        return -1;
    }
    if( kwargs )
    {
        PyObject* key;
        PyObject* value;
        Py_ssize_t pos = 0;
        while( PyDict_Next( kwargs, &pos, &key, &value ) )
        {
            if( PyObject_SetAttr( self, key, value ) < 0 )
            {
                return -1;
            }
        }
    }
    return 0;
}


static void Member_clear( Member* self )
{
    Py_CLEAR( self->default_handler );
    Py_CLEAR( self->validate_handler );
    Py_CLEAR( self->post_validate_handler );
    Py_CLEAR( self->post_setattr_handler );
}


static int Member_traverse( Member* self, visitproc visit, void* arg )
{
    Py_VISIT( self->default_handler );
    Py_VISIT( self->validate_handler );
    Py_VISIT( self->post_validate_handler );
    Py_VISIT( self->post_setattr_handler );
    return 0;
}


static void Member_dealloc( Member* self )
{
    PyObject_GC_UnTrack( self );
    Member_clear( self );
    self->ob_type->tp_free( ( PyObject* )self );
}


static inline PyObject* _get_handler( PyObject** ref )
{
    return newref( *ref ? *ref : Py_None );
}


static inline int _set_handler( PyObject** ref, PyObject* value )
{
    if( !value || value == Py_None )
    {
        PyObject* old = *ref;
        *ref = 0;
        Py_XDECREF( old );
        return 0;
    }
    if( !PyCallable_Check( value ) )
    {
        py_expected_type_fail( value, "callable" );
        return -1;
    }
    PyObject* old = *ref;
    *ref = newref( value );
    Py_XDECREF( old );
    return 0;
}


static PyObject* Member_get_default_handler( Member* self, void* ctxt )
{
    return _get_handler( &self->default_handler );
}


static int
Member_set_default_handler( Member* self, PyObject* value, void* ctxt )
{
    return _set_handler( &self->default_handler, value );
}


static PyObject* Member_get_validate_handler( Member* self, void* ctxt )
{
    return _get_handler( &self->validate_handler );
}


static int
Member_set_validate_handler( Member* self, PyObject* value, void* ctxt )
{
    return _set_handler( &self->validate_handler, value );
}


static PyObject* Member_get_post_validate_handler( Member* self, void* ctxt )
{
    return _get_handler( &self->post_validate_handler );
}


static int
Member_set_post_validate_handler( Member* self, PyObject* value, void* ctxt )
{
    return _set_handler( &self->post_validate_handler, value );
}


static PyObject* Member_get_post_setattr_handler( Member* self, void* ctxt )
{
    return _get_handler( &self->post_setattr_handler );
}


static int
Member_set_post_setattr_handler( Member* self, PyObject* value, void* ctxt )
{
    return _set_handler( &self->post_setattr_handler, value );
}


static PyGetSetDef Member_getset[] = {
    {"default_handler",
     ( getter )Member_get_default_handler,
     ( setter )Member_set_default_handler,
     "Get and set the default value handler for the member."},
    {"validate_handler",
     ( getter )Member_get_validate_handler,
     ( setter )Member_set_validate_handler,
     "Get and set the default value handler for the member."},
    {"post_validate_handler",
     ( getter )Member_get_post_validate_handler,
     ( setter )Member_set_post_validate_handler,
     "Get and set the default value handler for the member."},
    {"post_setattr_handler",
     ( getter )Member_get_post_setattr_handler,
     ( setter )Member_set_post_setattr_handler,
     "Get and set the default value handler for the member."},
    {0} // sentinel
};


PyTypeObject Member_Type = {
    PyObject_HEAD_INIT( &PyType_Type ) /* header */
    0,                                 /* ob_size */
    "atom.catom.Member",               /* tp_name */
    sizeof( Member ),                  /* tp_basicsize */
    0,                                 /* tp_itemsize */
    ( destructor )Member_dealloc,      /* tp_dealloc */
    ( printfunc )0,                    /* tp_print */
    ( getattrfunc )0,                  /* tp_getattr */
    ( setattrfunc )0,                  /* tp_setattr */
    ( cmpfunc )0,                      /* tp_compare */
    ( reprfunc )0,                     /* tp_repr */
    ( PyNumberMethods* )0,             /* tp_as_number */
    ( PySequenceMethods* )0,           /* tp_as_sequence */
    ( PyMappingMethods* )0,            /* tp_as_mapping */
    ( hashfunc )0,                     /* tp_hash */
    ( ternaryfunc )0,                  /* tp_call */
    ( reprfunc )0,                     /* tp_str */
    ( getattrofunc )0,                 /* tp_getattro */
    ( setattrofunc )0,                 /* tp_setattro */
    ( PyBufferProcs* )0,               /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_GC |
        Py_TPFLAGS_HAVE_VERSION_TAG,  /* tp_flags */
    0,                                /* Documentation string */
    ( traverseproc )Member_traverse,  /* tp_traverse */
    ( inquiry )Member_clear,          /* tp_clear */
    ( richcmpfunc )0,                 /* tp_richcompare */
    0,                                /* tp_weaklistoffset */
    ( getiterfunc )0,                 /* tp_iter */
    ( iternextfunc )0,                /* tp_iternext */
    ( struct PyMethodDef* )0,         /* tp_methods */
    ( struct PyMemberDef* )0,         /* tp_members */
    Member_getset,                    /* tp_getset */
    0,                                /* tp_base */
    0,                                /* tp_dict */
    ( descrgetfunc )0,                /* tp_descr_get */
    ( descrsetfunc )0,                /* tp_descr_set */
    0,                                /* tp_dictoffset */
    ( initproc )Member_init,          /* tp_init */
    ( allocfunc )PyType_GenericAlloc, /* tp_alloc */
    ( newfunc )PyType_GenericNew,     /* tp_new */
    ( freefunc )PyObject_GC_Del,      /* tp_free */
    ( inquiry )0,                     /* tp_is_gc */
    0,                                /* tp_bases */
    0,                                /* tp_mro */
    0,                                /* tp_cache */
    0,                                /* tp_subclasses */
    0,                                /* tp_weaklist */
    ( destructor )0                   /* tp_del */
};


int import_member()
{
    return PyType_Ready( &Member_Type );
}
