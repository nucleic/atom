/*-----------------------------------------------------------------------------
| Copyright (c) 2013, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include <map>
#include <iostream>
#include <sstream>
#include "pythonhelpers.h"
#include "atomref.h"
#include "catompointer.h"
#include "globalstatic.h"
#include "packagenaming.h"
#include "py23compat.h"

#ifdef __clang__
#pragma clang diagnostic ignored "-Wdeprecated-writable-strings"
#endif

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif

#define atomref_cast( o ) ( reinterpret_cast<AtomRef*>( o ) )


using namespace PythonHelpers;


typedef struct {
    PyObject_HEAD
    CAtomPointer pointer;  // constructed with placement new
} AtomRef;


namespace SharedAtomRef
{

typedef std::map<CAtom*, PyObjectPtr> RefMap;
GLOBAL_STATIC( RefMap, ref_map )


PyObject*
get( CAtom* atom )
{
    if( atom->has_atomref() )
        return ( *ref_map() )[ atom ].newref();
    PyObject* pyref = AtomRef_Type.tp_alloc( &AtomRef_Type, 0 );
    if( !pyref )
        return 0;
    // placement new since Python malloc'd and zero'd the struct
    new( &atomref_cast( pyref )->pointer ) CAtomPointer( atom );
    ( *ref_map() )[ atom ] = newref( pyref );
    atom->set_has_atomref( true );
    return pyref;
}


void
clear( CAtom* atom )
{
    ref_map()->erase( atom );
    atom->set_has_atomref( false );
}

}  // namespace SharedAtomRef


static PyObject*
AtomRef_new( PyTypeObject* type, PyObject* args, PyObject* kwargs )
{
    static char *kwlist[] = { "atom", 0 };
    PyObject* atom;
    if( !PyArg_ParseTupleAndKeywords( args, kwargs, "O:__new__", kwlist, &atom ) )
        return 0;
    if( !CAtom::TypeCheck( atom ) )
        return py_expected_type_fail( atom, "CAtom" );
    return SharedAtomRef::get( catom_cast( atom ) );
}


static void
AtomRef_dealloc( AtomRef* self )
{
    // manual destructor since Python malloc'd and zero'd the struct
    self->pointer.~CAtomPointer();
    Py_TYPE(self)->tp_free( pyobject_cast( self ) );
}


static PyObject*
AtomRef_call( AtomRef* self, PyObject* args, PyObject* kwargs )
{
    static char *kwlist[] = { 0 };
    if( !PyArg_ParseTupleAndKeywords( args, kwargs, ":__call__", kwlist ) )
        return 0;
    PyObject* obj = pyobject_cast( self->pointer.data() );
    return newref( obj ? obj : Py_None );
}


static PyObject*
AtomRef_repr( AtomRef* self )
{
    std::ostringstream ostr;
    ostr << "AtomRef(atom=";
    if( self->pointer.is_null() )
        ostr << "None";
    else
    {
        PyObject* obj = pyobject_cast( self->pointer.data() );
        PyObjectPtr repr( PyObject_Repr( obj ) );
        if( !repr )
            return 0;
        ostr << Py23Str_AS_STRING( repr.get() );
    }
    ostr << ")";
    return Py23Str_FromString( ostr.str().c_str() );
}


static PyObject*
AtomRef_sizeof( AtomRef* self, PyObject* args )
{
    Py_ssize_t size = Py_TYPE(self)->tp_basicsize;
    size += sizeof( CAtomPointer );
    return Py23Int_FromSsize_t( size );
}


static int
AtomRef__nonzero__( AtomRef* self )
{
    return self->pointer.is_null() ? 0 : 1;
}


PyNumberMethods AtomRef_as_number = {
     ( binaryfunc )0,                       /* nb_add */
     ( binaryfunc )0,                       /* nb_subtract */
     ( binaryfunc )0,                       /* nb_multiply */
#if PY_MAJOR_VERSION < 3
     ( binaryfunc )0,                       /* nb_divide */
#endif
     ( binaryfunc )0,                       /* nb_remainder */
     ( binaryfunc )0,                       /* nb_divmod */
     ( ternaryfunc )0,                      /* nb_power */
     ( unaryfunc )0,                        /* nb_negative */
     ( unaryfunc )0,                        /* nb_positive */
     ( unaryfunc )0,                        /* nb_absolute */
     ( inquiry )AtomRef__nonzero__          /* nb_nonzero, or nb_bool in python3 */
};


static PyMethodDef
AtomRef_methods[] = {
    { "__sizeof__", ( PyCFunction )AtomRef_sizeof, METH_NOARGS,
      "__sizeof__() -> size of object in memory, in bytes" },
    { 0 } // sentinel
};


PyTypeObject AtomRef_Type = {
    PyVarObject_HEAD_INIT( NULL, 0 )
    PACKAGE_TYPENAME( "atomref" ),          /* tp_name */
    sizeof( AtomRef ),                      /* tp_basicsize */
    0,                                      /* tp_itemsize */
    (destructor)AtomRef_dealloc,            /* tp_dealloc */
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
    (reprfunc)AtomRef_repr,                 /* tp_repr */
    (PyNumberMethods*)&AtomRef_as_number,   /* tp_as_number */
    (PySequenceMethods*)0,                  /* tp_as_sequence */
    (PyMappingMethods*)0,                   /* tp_as_mapping */
    (hashfunc)0,                            /* tp_hash */
    (ternaryfunc)AtomRef_call,              /* tp_call */
    (reprfunc)0,                            /* tp_str */
    (getattrofunc)0,                        /* tp_getattro */
    (setattrofunc)0,                        /* tp_setattro */
    (PyBufferProcs*)0,                      /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,                     /* tp_flags */
    0,                                      /* Documentation string */
    (traverseproc)0,                        /* tp_traverse */
    (inquiry)0,                             /* tp_clear */
    (richcmpfunc)0,                         /* tp_richcompare */
    0,                                      /* tp_weaklistoffset */
    (getiterfunc)0,                         /* tp_iter */
    (iternextfunc)0,                        /* tp_iternext */
    (struct PyMethodDef*)AtomRef_methods,   /* tp_methods */
    (struct PyMemberDef*)0,                 /* tp_members */
    0,                                      /* tp_getset */
    0,                                      /* tp_base */
    0,                                      /* tp_dict */
    (descrgetfunc)0,                        /* tp_descr_get */
    (descrsetfunc)0,                        /* tp_descr_set */
    0,                                      /* tp_dictoffset */
    (initproc)0,                            /* tp_init */
    (allocfunc)PyType_GenericAlloc,         /* tp_alloc */
    (newfunc)AtomRef_new,                   /* tp_new */
    (freefunc)0,                            /* tp_free */
    (inquiry)0,                             /* tp_is_gc */
    0,                                      /* tp_bases */
    0,                                      /* tp_mro */
    0,                                      /* tp_cache */
    0,                                      /* tp_subclasses */
    0,                                      /* tp_weaklist */
    (destructor)0                           /* tp_del */
};


int
import_atomref()
{
    if( PyType_Ready( &AtomRef_Type ) < 0 )
        return -1;
    return 0;
}
