/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include <cppy/cppy.h>
#include "member.h"


namespace atom
{

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
    Py_CLEAR( self->m_default );
    Py_CLEAR( self->m_validate );
    Py_CLEAR( self->m_post_validate );
    Py_CLEAR( self->m_post_setattr );
}


int Member_traverse( Member* self, visitproc visit, void* arg )
{
    Py_VISIT( self->m_default );
    Py_VISIT( self->m_validate );
    Py_VISIT( self->m_post_validate );
    Py_VISIT( self->m_post_setattr );
    return 0;
}


void Member_dealloc( Member* self )
{
    PyObject_GC_UnTrack( self );
    Member_clear( self );
    self->ob_type->tp_free( reinterpret_cast<PyObject*>( self ) );
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
    return get_handler( &self->m_default );
}


int Member_set_default_handler( Member* self, PyObject* value, void* ctxt )
{
    return set_handler( &self->m_default, value );
}


PyObject* Member_get_validate_handler( Member* self, void* ctxt )
{
    return get_handler( &self->m_validate );
}


int Member_set_validate_handler( Member* self, PyObject* value, void* ctxt )
{
    return set_handler( &self->m_validate, value );
}


PyObject* Member_get_post_validate_handler( Member* self, void* ctxt )
{
    return get_handler( &self->m_post_validate );
}


int
Member_set_post_validate_handler( Member* self, PyObject* value, void* ctxt )
{
    return set_handler( &self->m_post_validate, value );
}


PyObject* Member_get_post_setattr_handler( Member* self, void* ctxt )
{
    return get_handler( &self->m_post_setattr );
}


int Member_set_post_setattr_handler( Member* self, PyObject* value, void* ctxt )
{
    return set_handler( &self->m_post_setattr, value );
}


PyGetSetDef Member_getset[] = {
    { "default_handler",
      ( getter )Member_get_default_handler,
      ( setter )Member_set_default_handler,
      "Get and set the default value handler for the member." },
    { "validate_handler",
      ( getter )Member_get_validate_handler,
      ( setter )Member_set_validate_handler,
      "Get and set the default value handler for the member." },
    { "post_validate_handler",
      ( getter )Member_get_post_validate_handler,
      ( setter )Member_set_post_validate_handler,
      "Get and set the default value handler for the member." },
    { "post_setattr_handler",
      ( getter )Member_get_post_setattr_handler,
      ( setter )Member_set_post_setattr_handler,
      "Get and set the default value handler for the member." },
    { 0 } // sentinel
};

} // namespace


PyTypeObject Member::TypeObject = {
    PyObject_HEAD_INIT( &PyType_Type )
    0,                                 /* ob_size */
    "atom.catom.CMember",              /* tp_name */
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
    Py_TPFLAGS_DEFAULT
    | Py_TPFLAGS_BASETYPE
    | Py_TPFLAGS_HAVE_GC
    | Py_TPFLAGS_HAVE_VERSION_TAG,     /* tp_flags */
    0,                                 /* Documentation string */
    ( traverseproc )Member_traverse,   /* tp_traverse */
    ( inquiry )Member_clear,           /* tp_clear */
    ( richcmpfunc )0,                  /* tp_richcompare */
    0,                                 /* tp_weaklistoffset */
    ( getiterfunc )0,                  /* tp_iter */
    ( iternextfunc )0,                 /* tp_iternext */
    ( struct PyMethodDef* )0,          /* tp_methods */
    ( struct PyMemberDef* )0,          /* tp_members */
    Member_getset,                     /* tp_getset */
    0,                                 /* tp_base */
    0,                                 /* tp_dict */
    ( descrgetfunc )0,                 /* tp_descr_get */
    ( descrsetfunc )0,                 /* tp_descr_set */
    0,                                 /* tp_dictoffset */
    ( initproc )Member_init,           /* tp_init */
    ( allocfunc )PyType_GenericAlloc,  /* tp_alloc */
    ( newfunc )PyType_GenericNew,      /* tp_new */
    ( freefunc )PyObject_GC_Del,       /* tp_free */
    ( inquiry )0,                      /* tp_is_gc */
    0,                                 /* tp_bases */
    0,                                 /* tp_mro */
    0,                                 /* tp_cache */
    0,                                 /* tp_subclasses */
    0,                                 /* tp_weaklist */
    ( destructor )0                    /* tp_del */
};


bool Member::Ready()
{
    return PyType_Ready( &TypeObject ) == 0;
}


PyObject* Member::getDefault( PyObject* atom, PyObject* name )
{
    cppy::ptr result( Py_None, true );
    if( m_default )
    {
        cppy::ptr args( PyTuple_Pack( 2, atom, name ) );
        if( !args )
        {
            return 0;
        }
        result = PyObject_Call( m_default, args.get(), 0 );
        if( !result )
        {
            return 0;
        }
    }
    return validate( atom, name, result.get() );
}


PyObject* Member::validate( PyObject* atom, PyObject* name, PyObject* value )
{
    cppy::ptr result( value, true );
    if( m_validate )
    {
        cppy::ptr args( PyTuple_Pack( 3, atom, name, result.get() ) );
        if( !args )
        {
            return 0;
        }
        result = PyObject_Call( m_validate, args.get(), 0 );
        if( !result )
        {
            return 0;
        }
    }
    if( m_post_validate )
    {
        cppy::ptr args( PyTuple_Pack( 3, atom, name, result.get() ) );
        if( !args )
        {
            return 0;
        }
        result = PyObject_Call( m_post_validate, args.get(), 0 );
        if( !result )
        {
            return 0;
        }
    }
    return result.release();
}


int Member::postSetAttr( PyObject* atom, PyObject* name, PyObject* value )
{
    if( m_post_setattr )
    {
        cppy::ptr args( PyTuple_Pack( 3, atom, name, value ) );
        if( !args )
        {
            return 0;
        }
        cppy::ptr result( PyObject_Call( m_post_setattr, args.get(), 0 ) );
        if( !result )
        {
            return -1;
        }
    }
    return 0;
}

} // namespace atom
