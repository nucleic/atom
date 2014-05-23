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
#include "descriptor.h"
#include "member.h"
#include "utils.h"

#include "ignoredwarnings.h"


using namespace PythonHelpers;


static PyObject* atom_descr_str;


static PyObject*  // new ref on success, null and exception on failure
lookup_descriptor( PyTypeObject* type )
{
    PyObject* py_descr = PyDict_GetItem( type->tp_dict, atom_descr_str );
    if( py_descr )
    {
        if( !Descriptor_Check( py_descr ) )
        {
            return py_bad_internal_call( "atom descriptor has invalid type" );
        }
        return newref( py_descr );
    }
    return py_bad_internal_call( "atom type has no descriptor" );
}


static PyObject*
CAtom_new( PyTypeObject* type, PyObject* args, PyObject* kwargs )
{
    PyObjectPtr descr_ptr( lookup_descriptor( type ) );
    if( !descr_ptr )
    {
        return 0;
    }
    PyObjectPtr self_ptr( PyType_GenericNew( type, args, kwargs ) );
    if( !self_ptr )
    {
        return 0;
    }
    CAtom* atom = reinterpret_cast<CAtom*>( self_ptr.get() );
    Descriptor* descr = reinterpret_cast<Descriptor*>( descr_ptr.get() );
    if( descr->slot_count > 0 )
    {
        size_t size = sizeof( PyObject* ) * descr->slot_count;
        void* slots = PyObject_Malloc( size );
        if( !slots )
        {
            return PyErr_NoMemory();
        }
        memset( slots, 0, size );
        atom->slots = reinterpret_cast<PyObject**>( slots );
        atom->allocated = descr->slot_count;
    }
    atom->descriptor = reinterpret_cast<Descriptor*>( descr_ptr.release() );
    utils::set_flag( atom, CAtom::NotificationsEnabled );
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
    uint16_t count = self->descriptor->slot_count;
    for( uint16_t i = 0; i < count; ++i )
    {
        Py_CLEAR( self->slots[ i ] );
    }
    Py_CLEAR( self->descriptor );
}


static int
CAtom_traverse( CAtom* self, visitproc visit, void* arg )
{
    uint16_t count = self->descriptor->slot_count;
    for( uint16_t i = 0; i < count; ++i )
    {
        Py_VISIT( self->slots[ i ] );
    }
    Py_VISIT( self->descriptor );
    return 0;
}


static void
CAtom_dealloc( CAtom* self )
{
    PyObject_GC_UnTrack( self );
    CAtom_clear( self );
    PyObject_Free( self->slots );
    self->ob_type->tp_free( reinterpret_cast<PyObject*>( self ) );
}


static PyObject*
CAtom_getattro( CAtom* self, PyStringObject* name )
{
    Member* member = Descriptor_LookupMember( self->descriptor, name );
    if( member )
    {
        if( member->slot_index >= self->allocated )
        {
            return py_bad_internal_call( "member slot index out of range" );
        }
        PyObject* value = self->slots[ member->slot_index ];
        return newref( value ? value : Py_None );
    }
    PyObject* py_self = reinterpret_cast<PyObject*>( self );
    PyObject* py_name = reinterpret_cast<PyObject*>( name );
    return PyObject_GenericGetAttr( py_self, py_name );
}


static int
CAtom_setattro( CAtom* self, PyStringObject* name, PyObject* value )
{
    Member* member = Descriptor_LookupMember( self->descriptor, name );
    if( member )
    {
        if( member->slot_index >= self->allocated )
        {
            py_bad_internal_call( "member slot index out of range" );
            return -1;
        }
        PyObject* old = self->slots[ member->slot_index ];
        self->slots[ member->slot_index ] = value;
        Py_XINCREF( value );
        Py_XDECREF( old );
        return 0;
    }
    PyObject* py_self = reinterpret_cast<PyObject*>( self );
    PyObject* py_name = reinterpret_cast<PyObject*>( name );
    return PyObject_GenericSetAttr( py_self, py_name, value );
}


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
    (struct PyMethodDef*)0,                 /* tp_methods */
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
    if( PyType_Ready( &CAtom_Type ) < 0 )
    {
        return -1;
    }
    atom_descr_str = PyString_FromString( "_[atom descriptor]" );
    if( !atom_descr_str )
    {
        return -1;
    }
    return 0;
}
