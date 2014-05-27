/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include <Python.h>
#include "pythonhelpers.h"
#include "inttypes.h"
#include "catom.h"
#include "class_map.h"
#include "member.h"
#include "static_strings.h"

#include "ignoredwarnings.h"


using namespace PythonHelpers;


static PyObject*  // new ref on success, null and exception on failure
lookup_class_map( PyTypeObject* type )
{
    PyObject* py_map = PyDict_GetItem( type->tp_dict, StaticStrings::ClassMap );
    if( py_map )
    {
        if( !ClassMap_Check( py_map ) )
        {
            return py_bad_internal_call( "class map has invalid type" );
        }
        return newref( py_map );
    }
    return py_bad_internal_call( "atom type has no class map" );
}


static PyObject*
CAtom_new( PyTypeObject* type, PyObject* args, PyObject* kwargs )
{
    PyObjectPtr map_ptr( lookup_class_map( type ) );
    if( !map_ptr )
    {
        return 0;
    }
    PyObjectPtr self_ptr( PyType_GenericNew( type, args, kwargs ) );
    if( !self_ptr )
    {
        return 0;
    }
    CAtom* atom = ( CAtom* )self_ptr.get();
    ClassMap* map = ( ClassMap* )map_ptr.get();
    uint32_t count = ClassMap_MemberCount( map );
    if( count > 0 )
    {
        size_t memsize = sizeof( PyObject* ) * count;
        void* slotmem = PyObject_Malloc( memsize );
        if( !slotmem )
        {
            return PyErr_NoMemory();
        }
        memset( slotmem, 0, memsize );
        atom->slots = ( PyObject** )slotmem;
    }
    atom->class_map = ( ClassMap* )map_ptr.release();
    return self_ptr.release();
}


static int
CAtom_init( PyObject* self, PyObject* args, PyObject* kwargs )
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


static void
CAtom_clear( CAtom* self )
{
    uint32_t count = ClassMap_MemberCount( self->class_map );
    for( uint32_t i = 0; i < count; ++i )
    {
        Py_CLEAR( self->slots[ i ] );
    }
    Py_CLEAR( self->class_map );
}


static int
CAtom_traverse( CAtom* self, visitproc visit, void* arg )
{
    uint32_t count = ClassMap_MemberCount( self->class_map );
    for( uint32_t i = 0; i < count; ++i )
    {
        Py_VISIT( self->slots[ i ] );
    }
    Py_VISIT( self->class_map );
    return 0;
}


static void
CAtom_dealloc( CAtom* self )
{
    PyObject_GC_UnTrack( self );
    CAtom_clear( self );
    PyObject_Free( self->slots );
    self->ob_type->tp_free( ( PyObject* )self );
}


static PyObject*
CAtom_getattro( CAtom* self, PyStringObject* name )
{
    // no smart pointers in this function, save stack setup costs!
    uint32_t index;
    Member* member = 0;
    ClassMap_LookupMember( self->class_map, name, &member, &index );
    if( member )
    {
        PyObject* value = self->slots[ index ];
        if( value )
        {
            return newref( value );
        }
        value = Member_Default( member, self, name );
        if( !value )
        {
            return 0;
        }
        PyObject* valid = Member_Validate( member, self, name, Py_None, value );
        Py_DECREF( value );
        if( !valid )
        {
            return 0;
        }
        self->slots[ index ] = valid;
        // XXX emit "create" notification
        return newref( valid );
    }
    return PyObject_GenericGetAttr( ( PyObject* )self, ( PyObject* )name );
}


static int
CAtom_setattro( CAtom* self, PyStringObject* name, PyObject* value )
{
    uint32_t index;
    Member* member = 0;
    ClassMap_LookupMember( self->class_map, name, &member, &index );
    if( member )
    {
        PyObject* old = self->slots[ index ];
        if( value == old )
        {
            return 0;
        }
        if( !value )
        {
            self->slots[ index ] = 0;
            // XXX emit "delete" notification
            Py_DECREF( old );
            return 0;
        }
        old = old ? old : newref( Py_None );
        value = Member_Validate( member, self, name, old, value );
        if( !value )
        {
            return 0;
        }
        self->slots[ index ] = value;
        if( Member_PostSetAttr( member, self, name, old, value ) < 0 )
        {
            Py_DECREF( old );
            return -1;
        }
        // XXX emit "create" or "update" notification
        Py_DECREF( old );
        return 0;
    }
    return PyObject_GenericSetAttr( ( PyObject* )self, ( PyObject* )name, value );
}


static PyObject*
CAtom_sizeof( CAtom* self, PyObject* args )
{
    Py_ssize_t size = self->ob_type->tp_basicsize;
    size += sizeof( PyObject* ) * ClassMap_MemberCount( self->class_map );
    return PyInt_FromSsize_t( size );
}


static PyMethodDef
CAtom_methods[] = {
    { "__sizeof__", ( PyCFunction )CAtom_sizeof, METH_NOARGS,
      "__sizeof__() -> size of object in memory, in bytes" },
    { 0 } // sentinel
};


PyTypeObject CAtom_Type = {
    PyObject_HEAD_INIT( &PyType_Type )
    0,                                      /* ob_size */
    "atom.catom.CAtom",                     /* tp_name */
    sizeof( CAtom ),                        /* tp_basicsize */
    0,                                      /* tp_itemsize */
    (destructor)CAtom_dealloc,              /* tp_dealloc */
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
    (getattrofunc)CAtom_getattro,           /* tp_getattro */
    (setattrofunc)CAtom_setattro,           /* tp_setattro */
    (PyBufferProcs*)0,                      /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE|Py_TPFLAGS_HAVE_GC|Py_TPFLAGS_HAVE_VERSION_TAG, /* tp_flags */
    0,                                      /* Documentation string */
    (traverseproc)CAtom_traverse,           /* tp_traverse */
    (inquiry)CAtom_clear,                   /* tp_clear */
    (richcmpfunc)0,                         /* tp_richcompare */
    0,                                      /* tp_weaklistoffset */
    (getiterfunc)0,                         /* tp_iter */
    (iternextfunc)0,                        /* tp_iternext */
    (struct PyMethodDef*)CAtom_methods,     /* tp_methods */
    (struct PyMemberDef*)0,                 /* tp_members */
    0,                                      /* tp_getset */
    0,                                      /* tp_base */
    0,                                      /* tp_dict */
    (descrgetfunc)0,                        /* tp_descr_get */
    (descrsetfunc)0,                        /* tp_descr_set */
    0,                                      /* tp_dictoffset */
    (initproc)CAtom_init,                   /* tp_init */
    (allocfunc)PyType_GenericAlloc,         /* tp_alloc */
    (newfunc)CAtom_new,                     /* tp_new */
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
import_catom()
{
    return PyType_Ready( &CAtom_Type );
}
