/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include <cppy/cppy.h>
#include "member.h"
#include "null_object.h"


PyObject* Member_Default( Member* member, Atom* atom, PyStringObject* name )
{
    cppy::ptr result( Py_None, true );
    if( member->default_handler )
    {
        cppy::ptr args( PyTuple_Pack( 2, atom, name ) );
        if( !args )
        {
            return 0;
        }
        result = PyObject_Call( member->default_handler, args.get(), 0 );
        if( !result )
        {
            return 0;
        }
    }
    return Member_Validate( member, atom, name, result.get() );
}


PyObject* Member_Validate( Member* member,
                           Atom* atom,
                           PyStringObject* name,
                           PyObject* value )
{
    cppy::ptr result( value, true );
    if( member->validate_handler )
    {
        cppy::ptr args( PyTuple_Pack( 3, atom, name, result.get() ) );
        if( !args )
        {
            return 0;
        }
        result = PyObject_Call( member->validate_handler, args.get(), 0 );
        if( !result )
        {
            return 0;
        }
    }
    if( member->post_validate_handler )
    {
        cppy::ptr args( PyTuple_Pack( 3, atom, name, result.get() ) );
        if( !args )
        {
            return 0;
        }
        result = PyObject_Call( member->post_validate_handler, args.get(), 0 );
        if( !result )
        {
            return 0;
        }
    }
    return result.release();
}


int Member_PostSetAttr( Member* member,
                        Atom* atom,
                        PyStringObject* name,
                        PyObject* value )
{
    if( member->post_setattr_handler )
    {
        cppy::ptr args( PyTuple_Pack( 3, atom, name, value ) );
        if( !args )
        {
            return 0;
        }
        cppy::ptr result(
            PyObject_Call( member->post_setattr_handler, args.get(), 0 ) );
        if( !result )
        {
            return -1;
        }
    }
    return 0;
}


namespace
{

int Member_init( PyObject* self, PyObject* args, PyObject* kwargs )
{
    if( PyTuple_GET_SIZE( args ) > 0 )
    {
        cppy::type_error( "__init__() takes no positional arguments" );
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


void Member_clear( Member* self )
{
    Py_CLEAR( self->default_handler );
    Py_CLEAR( self->validate_handler );
    Py_CLEAR( self->post_validate_handler );
    Py_CLEAR( self->post_setattr_handler );
}


int Member_traverse( Member* self, visitproc visit, void* arg )
{
    Py_VISIT( self->default_handler );
    Py_VISIT( self->validate_handler );
    Py_VISIT( self->post_validate_handler );
    Py_VISIT( self->post_setattr_handler );
    return 0;
}


void Member_dealloc( Member* self )
{
    PyObject_GC_UnTrack( self );
    Member_clear( self );
    self->ob_type->tp_free( ( PyObject* )self );
}


inline PyObject* get_handler( PyObject** ref )
{
    return cppy::incref( *ref ? *ref : Py_None );
}


inline int set_handler( PyObject** ref, PyObject* handler )
{
    if( !handler || handler == Py_None )
    {
        cppy::clear( ref );
        return 0;
    }
    if( !PyCallable_Check( handler ) )
    {
        cppy::type_error( handler, "callable" );
        return -1;
    }
    cppy::replace( ref, handler );
    return 0;
}


PyObject* Member_get_default_handler( Member* self, void* ctxt )
{
    return get_handler( &self->default_handler );
}


int Member_set_default_handler( Member* self, PyObject* value, void* ctxt )
{
    return set_handler( &self->default_handler, value );
}


PyObject* Member_get_validate_handler( Member* self, void* ctxt )
{
    return get_handler( &self->validate_handler );
}


int Member_set_validate_handler( Member* self, PyObject* value, void* ctxt )
{
    return set_handler( &self->validate_handler, value );
}


PyObject* Member_get_post_validate_handler( Member* self, void* ctxt )
{
    return get_handler( &self->post_validate_handler );
}


int
Member_set_post_validate_handler( Member* self, PyObject* value, void* ctxt )
{
    return set_handler( &self->post_validate_handler, value );
}


PyObject* Member_get_post_setattr_handler( Member* self, void* ctxt )
{
    return get_handler( &self->post_setattr_handler );
}


int Member_set_post_setattr_handler( Member* self, PyObject* value, void* ctxt )
{
    return set_handler( &self->post_setattr_handler, value );
}


PyGetSetDef Member_getset[] = {
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

} // namespace


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
