/*-----------------------------------------------------------------------------
| Copyright (c) 2013, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include <iostream>
#include <sstream>
#include "pythonhelpers.h"
#include "catom.h"
#include "catompointer.h"


using namespace PythonHelpers;


typedef struct {
    PyObject_HEAD
    CAtomPointer* pointer;
} AtomRef;


static PyObject*
AtomRef_new( PyTypeObject* type, PyObject* args, PyObject* kwargs )
{
    static char *kwlist[] = { "atom", 0 };
    PyObject* atom;
    if( !PyArg_ParseTupleAndKeywords( args, kwargs, "O:__new__", kwlist, &atom ) )
        return 0;
    if( !CAtom::TypeCheck( atom ) )
        return py_expected_type_fail( atom, "CAtom" );
    PyObject* self = PyType_GenericNew( type, args, kwargs );
    if( !self )
        return 0;
    AtomRef* cself = reinterpret_cast<AtomRef*>( self );
    cself->pointer = new CAtomPointer( catom_cast( atom ) );
    return self;
}


static void
AtomRef_dealloc( AtomRef* self )
{
    delete self->pointer;
    self->pointer = 0;
    self->ob_type->tp_free( pyobject_cast( self ) );
}


static PyObject*
AtomRef_call( AtomRef* self, PyObject* args, PyObject* kwargs )
{
    static char *kwlist[] = { 0 };
    if( !PyArg_ParseTupleAndKeywords( args, kwargs, ":__call__", kwlist ) )
        return 0;
    PyObject* obj = pyobject_cast( self->pointer->data() );
    if( obj )
        return newref( obj );
    Py_RETURN_NONE;
}


static PyObject*
AtomRef_repr( AtomRef* self )
{
    std::ostringstream ostr;
    ostr << "AtomRef(object=";
    if( self->pointer->is_null() )
        ostr << "None";
    else
    {
        PyObject* obj = pyobject_cast( self->pointer->data() );
        PyObjectPtr repr( PyObject_Repr( obj ) );
        if( !repr )
            return 0;
        ostr << PyString_AsString( repr.get() );
    }
    ostr << ")";
    return PyString_FromString( ostr.str().c_str() );
}


static PyObject*
AtomRef_sizeof( AtomRef* self, PyObject* args )
{
    Py_ssize_t size = self->ob_type->tp_basicsize;
    size += sizeof( CAtomPointer );
    return PyInt_FromSsize_t( size );
}


static int
AtomRef__nonzero__( AtomRef* self )
{
    if( self->pointer->is_null() )
        return 0;
    return 1;
}


PyNumberMethods AtomRef_as_number = {
     ( binaryfunc )0,                       /* nb_add */
     ( binaryfunc )0,                       /* nb_subtract */
     ( binaryfunc )0,                       /* nb_multiply */
     ( binaryfunc )0,                       /* nb_divide */
     ( binaryfunc )0,                       /* nb_remainder */
     ( binaryfunc )0,                       /* nb_divmod */
     ( ternaryfunc )0,                      /* nb_power */
     ( unaryfunc )0,                        /* nb_negative */
     ( unaryfunc )0,                        /* nb_positive */
     ( unaryfunc )0,                        /* nb_absolute */
     ( inquiry )AtomRef__nonzero__          /* nb_nonzero */
};


static PyMethodDef
AtomRef_methods[] = {
    { "__sizeof__", ( PyCFunction )AtomRef_sizeof, METH_NOARGS,
      "__sizeof__() -> size of object in memory, in bytes" },
    { 0 } // sentinel
};


PyTypeObject AtomRef_Type = {
    PyObject_HEAD_INIT( 0 )
    0,                                      /* ob_size */
    "catom.AtomRef",                        /* tp_name */
    sizeof( AtomRef ),                      /* tp_basicsize */
    0,                                      /* tp_itemsize */
    (destructor)AtomRef_dealloc,            /* tp_dealloc */
    (printfunc)0,                           /* tp_print */
    (getattrfunc)0,                         /* tp_getattr */
    (setattrfunc)0,                         /* tp_setattr */
    (cmpfunc)0,                             /* tp_compare */
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
