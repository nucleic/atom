/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include <Python.h>
#include "pythonhelpers.h"
#include "catom.h"
#include "member.h"
#include "default_value_behavior.h"

#include "ignoredwarnings.h"


using namespace PythonHelpers;


/*

PyObject*  // new ref on success, null on failure
Member_Validate( Member* member, CAtom* atom, PyStringObject* name, PyObject*
old, PyObject* value )
{
    if( Member_TestFlag( member, Member::ObjectValidate ) )
    {
        PyObjectPtr m_name( PySequence_Concat( StaticStrings::ValidatePrefix, (
PyObject* )name ) );
        if( !m_name )
        {
            return 0;
        }
        PyObjectPtr method( PyObject_GetAttr( ( PyObject* )atom, m_name.get() )
);
        if( !method )
        {
            return 0;
        }
        PyTuplePtr args( PyTuple_New( 1 ) );
        if( !args )
        {
            return 0;
        }
        args.initialize( 0, newref( value ) );
        return PyObject_Call( method.get(), args.get(), 0 );
    }
    if( Member_TestFlag( member, Member::MemberValidate ) )
    {
        PyObjectPtr method( PyObject_GetAttr( ( PyObject* )member,
StaticStrings::Validate ) );
        if( !method )
        {
            return 0;
        }
        PyTuplePtr args( PyTuple_New( 3 ) );
        if( !args )
        {
            return 0;
        }
        args.initialize( 0, newref( ( PyObject* )atom ) );
        args.initialize( 1, newref( ( PyObject* )name ) );
        args.initialize( 2, newref( value ) );
        return PyObject_Call( method.get(), args.get(), 0 );
    }
    return newref( value );
}


int  // 0 on success, -1 on failure
Member_PostSetAttr( Member* member, CAtom* atom, PyStringObject* name, PyObject*
old, PyObject* value )
{
    if( Member_TestFlag( member, Member::ObjectPostSetattr ) )
    {
        PyObjectPtr m_name( PySequence_Concat( StaticStrings::PostSetattrPrefix,
( PyObject* )name ) );
        if( !m_name )
        {
            return -1;
        }
        PyObjectPtr method( PyObject_GetAttr( ( PyObject* )atom, m_name.get() )
);
        if( !method )
        {
            return -1;
        }
        PyTuplePtr args( PyTuple_New( 1 ) );
        if( !args )
        {
            return -1;
        }
        args.initialize( 0, newref( value ) );
        PyObjectPtr res( PyObject_Call( method.get(), args.get(), 0 ) );
        if( !res )
        {
            return -1;
        }
        return 0;
    }
    if( Member_TestFlag( member, Member::MemberPostSetattr ) )
    {
        PyObjectPtr method( PyObject_GetAttr( ( PyObject* )member,
StaticStrings::PostSetattr ) );
        if( !method )
        {
            return -1;
        }
        PyTuplePtr args( PyTuple_New( 3 ) );
        if( !args )
        {
            return -1;
        }
        args.initialize( 0, newref( ( PyObject* )atom ) );
        args.initialize( 1, newref( ( PyObject* )name ) );
        args.initialize( 2, newref( value ) );
        PyObjectPtr res( PyObject_Call( method.get(), args.get(), 0 ) );
        if( !res )
        {
            return -1;
        }
        return 0;
    }
    return 0;
}

*/


PyObject* Member_GetAttr( Member* member,
                          CAtom* atom,
                          PyStringObject* name,
                          PyObject** slot )
{
    PyObject* value = *slot;
    if( value )
    {
        return newref( value );
    }
    value = Member_DefaultValue( member, atom, name );
    if( !value )
    {
        return 0;
    }
    *slot = value;
    return newref( value );
}


int Member_SetAttr( Member* member,
                    CAtom* atom,
                    PyStringObject* name,
                    PyObject* value,
                    PyObject** slot )
{
    PyObject* old = *slot;
    *slot = value;
    Py_XINCREF( value );
    Py_XDECREF( old );
    return 0;
}


static void Member_clear( Member* self )
{
    Py_CLEAR( self->metadata );
}


static int Member_traverse( Member* self, visitproc visit, void* arg )
{
    Py_VISIT( self->metadata );
    return 0;
}


static void Member_dealloc( Member* self )
{
    PyObject_GC_UnTrack( self );
    Member_clear( self );
    self->ob_type->tp_free( ( PyObject* )self );
}


PyTypeObject Member_Type = {
    PyObject_HEAD_INIT( &PyType_Type )0, /* ob_size */
    "atom.catom.Member",                 /* tp_name */
    sizeof( Member ),                    /* tp_basicsize */
    0,                                   /* tp_itemsize */
    ( destructor )Member_dealloc,        /* tp_dealloc */
    ( printfunc )0,                      /* tp_print */
    ( getattrfunc )0,                    /* tp_getattr */
    ( setattrfunc )0,                    /* tp_setattr */
    ( cmpfunc )0,                        /* tp_compare */
    ( reprfunc )0,                       /* tp_repr */
    ( PyNumberMethods* )0,               /* tp_as_number */
    ( PySequenceMethods* )0,             /* tp_as_sequence */
    ( PyMappingMethods* )0,              /* tp_as_mapping */
    ( hashfunc )0,                       /* tp_hash */
    ( ternaryfunc )0,                    /* tp_call */
    ( reprfunc )0,                       /* tp_str */
    ( getattrofunc )0,                   /* tp_getattro */
    ( setattrofunc )0,                   /* tp_setattro */
    ( PyBufferProcs* )0,                 /* tp_as_buffer */
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
    0,                                /* tp_getset */
    0,                                /* tp_base */
    0,                                /* tp_dict */
    ( descrgetfunc )0,                /* tp_descr_get */
    ( descrsetfunc )0,                /* tp_descr_set */
    0,                                /* tp_dictoffset */
    ( initproc )0,                    /* tp_init */
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
