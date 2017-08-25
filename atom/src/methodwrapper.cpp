/*-----------------------------------------------------------------------------
| Copyright (c) 2013, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include "methodwrapper.h"
#include "catom.h"
#include "catompointer.h"


using namespace PythonHelpers;


typedef struct {
    PyObject_HEAD
    PyObject* im_func;
    PyObject* im_selfref;
} MethodWrapper;


typedef struct {
    PyObject_HEAD
    PyObject* im_func;
    CAtomPointer pointer;  // constructed with placement new
} AtomMethodWrapper;


/*-----------------------------------------------------------------------------
| MethodWrapper
|----------------------------------------------------------------------------*/
static int
MethodWrapper_Check( PyObject* obj );


static void
MethodWrapper_dealloc( MethodWrapper* self )
{
    Py_CLEAR( self->im_selfref );
    Py_CLEAR( self->im_func );
    Py_TYPE(self)->tp_free( pyobject_cast( self ) );
}


static PyObject*
MethodWrapper__call__( MethodWrapper* self, PyObject* args, PyObject* kwargs )
{
    PyObject* im_self = PyWeakref_GET_OBJECT( self->im_selfref );
    if( im_self != Py_None )
    {
        #if PY_MAJOR_VERSION < 3
            PyObject* type = pyobject_cast( im_self->ob_type );
            PyObjectPtr method( PyMethod_New( self->im_func, im_self, type ) );
        #else
            PyObjectPtr method( PyMethod_New( self->im_func, im_self ) );
        #endif
        if( !method )
            return 0;
        return PyObject_Call( method.get(), args, kwargs );
    }
    Py_RETURN_NONE;
}


static PyObject*
MethodWrapper_richcompare( MethodWrapper* self, PyObject* other, int op )
{
    if( op == Py_EQ )
    {
        if( PyMethod_Check( other ) && PyMethod_GET_SELF( other ) )
        {
            if( ( self->im_func == PyMethod_GET_FUNCTION( other ) ) &&
                ( PyWeakref_GET_OBJECT( self->im_selfref ) == PyMethod_GET_SELF( other ) ) )
                Py_RETURN_TRUE;
            Py_RETURN_FALSE;
        }
        else if( MethodWrapper_Check( other ) )
        {
            MethodWrapper* wrapper = reinterpret_cast<MethodWrapper*>( other );
            if( ( self->im_func == wrapper->im_func ) &&
                ( self->im_selfref == wrapper->im_selfref ) )
                Py_RETURN_TRUE;
            Py_RETURN_FALSE;
        }
        else
            Py_RETURN_FALSE;
    }
    Py_RETURN_NOTIMPLEMENTED;
}


static int
MethodWrapper__nonzero__( MethodWrapper* self )
{
    if( PyWeakref_GET_OBJECT( self->im_selfref ) != Py_None )
        return 1;
    return 0;
}


PyNumberMethods MethodWrapper_as_number = {
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
     ( inquiry )MethodWrapper__nonzero__    /* nb_nonzero */
};


PyTypeObject MethodWrapper_Type = {
    PyVarObject_HEAD_INIT( &PyType_Type, 0 )
    //0,                                      /* ob_size */
    "MethodWrapper",                        /* tp_name */
    sizeof( MethodWrapper ),                /* tp_basicsize */
    0,                                      /* tp_itemsize */
    (destructor)MethodWrapper_dealloc,      /* tp_dealloc */
    (printfunc)0,                           /* tp_print */
    (getattrfunc)0,                         /* tp_getattr */
    (setattrfunc)0,                         /* tp_setattr */
    (cmpfunc)0,                             /* tp_compare */
    (reprfunc)0,                            /* tp_repr */
    (PyNumberMethods*)&MethodWrapper_as_number, /* tp_as_number */
    (PySequenceMethods*)0,                  /* tp_as_sequence */
    (PyMappingMethods*)0,                   /* tp_as_mapping */
    (hashfunc)0,                            /* tp_hash */
    (ternaryfunc)MethodWrapper__call__,     /* tp_call */
    (reprfunc)0,                            /* tp_str */
    (getattrofunc)0,                        /* tp_getattro */
    (setattrofunc)0,                        /* tp_setattro */
    (PyBufferProcs*)0,                      /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,                     /* tp_flags */
    0,                                      /* Documentation string */
    (traverseproc)0,                        /* tp_traverse */
    (inquiry)0,                             /* tp_clear */
    (richcmpfunc)MethodWrapper_richcompare, /* tp_richcompare */
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
    (newfunc)0,                             /* tp_new */
    (freefunc)PyObject_Del,                 /* tp_free */
    (inquiry)0,                             /* tp_is_gc */
    0,                                      /* tp_bases */
    0,                                      /* tp_mro */
    0,                                      /* tp_cache */
    0,                                      /* tp_subclasses */
    0,                                      /* tp_weaklist */
    (destructor)0                           /* tp_del */
};


static int
MethodWrapper_Check( PyObject* obj )
{
    return PyObject_TypeCheck( obj, &MethodWrapper_Type );
}


/*-----------------------------------------------------------------------------
| AtomMethodWrapper
|----------------------------------------------------------------------------*/
static int
AtomMethodWrapper_Check( PyObject* obj );


static void
AtomMethodWrapper_dealloc( AtomMethodWrapper* self )
{
    Py_CLEAR( self->im_func );
    // manual destructor since Python malloc'd and zero'd the struct
    self->pointer.~CAtomPointer();
    Py_TYPE(self)->tp_free( pyobject_cast( self ) );
}


static PyObject*
AtomMethodWrapper__call__( AtomMethodWrapper* self, PyObject* args, PyObject* kwargs )
{
    if( self->pointer.data() )
    {
        PyObject* im_self = pyobject_cast( self->pointer.data() );
        #if PY_MAJOR_VERSION < 3
            PyObject* type = pyobject_cast( im_self->ob_type );
            PyObjectPtr method( PyMethod_New( self->im_func, im_self, type ) );
        #else
            PyObjectPtr method( PyMethod_New( self->im_func, im_self ) );
        #endif
        if( !method )
            return 0;
        return PyObject_Call( method.get(), args, kwargs );
    }
    Py_RETURN_NONE;
}


static PyObject*
AtomMethodWrapper_richcompare( AtomMethodWrapper* self, PyObject* other, int op )
{
    if( op == Py_EQ )
    {
        if( PyMethod_Check( other ) && PyMethod_GET_SELF( other ) )
        {
            if( ( self->im_func == PyMethod_GET_FUNCTION( other ) ) &&
                ( pyobject_cast( self->pointer.data() ) == PyMethod_GET_SELF( other ) ) )
                Py_RETURN_TRUE;
            Py_RETURN_FALSE;
        }
        else if( AtomMethodWrapper_Check( other ) )
        {
            AtomMethodWrapper* wrapper = reinterpret_cast<AtomMethodWrapper*>( other );
            if( ( self->im_func == wrapper->im_func ) &&
                ( self->pointer.data() == wrapper->pointer.data() ) )
                Py_RETURN_TRUE;
            Py_RETURN_FALSE;
        }
        else
            Py_RETURN_FALSE;
    }
    Py_RETURN_NOTIMPLEMENTED;
}


static int
AtomMethodWrapper__nonzero__( AtomMethodWrapper* self )
{
    if( self->pointer.data() )
        return 1;
    return 0;
}


PyNumberMethods AtomMethodWrapper_as_number = {
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
     ( inquiry )AtomMethodWrapper__nonzero__ /* nb_nonzero */
};


PyTypeObject AtomMethodWrapper_Type = {
    PyVarObject_HEAD_INIT( &PyType_Type, 0 )
    //0,                                      /* ob_size */
    "AtomMethodWrapper",                    /* tp_name */
    sizeof( AtomMethodWrapper ),            /* tp_basicsize */
    0,                                      /* tp_itemsize */
    (destructor)AtomMethodWrapper_dealloc,  /* tp_dealloc */
    (printfunc)0,                           /* tp_print */
    (getattrfunc)0,                         /* tp_getattr */
    (setattrfunc)0,                         /* tp_setattr */
    (cmpfunc)0,                             /* tp_compare */
    (reprfunc)0,                            /* tp_repr */
    (PyNumberMethods*)&AtomMethodWrapper_as_number, /* tp_as_number */
    (PySequenceMethods*)0,                  /* tp_as_sequence */
    (PyMappingMethods*)0,                   /* tp_as_mapping */
    (hashfunc)0,                            /* tp_hash */
    (ternaryfunc)AtomMethodWrapper__call__, /* tp_call */
    (reprfunc)0,                            /* tp_str */
    (getattrofunc)0,                        /* tp_getattro */
    (setattrofunc)0,                        /* tp_setattro */
    (PyBufferProcs*)0,                      /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,                     /* tp_flags */
    0,                                      /* Documentation string */
    (traverseproc)0,                        /* tp_traverse */
    (inquiry)0,                             /* tp_clear */
    (richcmpfunc)AtomMethodWrapper_richcompare, /* tp_richcompare */
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
    (newfunc)0,                             /* tp_new */
    (freefunc)PyObject_Del,                 /* tp_free */
    (inquiry)0,                             /* tp_is_gc */
    0,                                      /* tp_bases */
    0,                                      /* tp_mro */
    0,                                      /* tp_cache */
    0,                                      /* tp_subclasses */
    0,                                      /* tp_weaklist */
    (destructor)0                           /* tp_del */
};


static int
AtomMethodWrapper_Check( PyObject* obj )
{
    return PyObject_TypeCheck( obj, &AtomMethodWrapper_Type );
}


/*-----------------------------------------------------------------------------
| External API
|----------------------------------------------------------------------------*/
PyObject*
MethodWrapper_New( PyObject* method )
{
    if( !PyMethod_Check( method ) )
        return py_expected_type_fail( method, "MethodType" );
    if( !PyMethod_GET_SELF( method ) )
        return py_type_fail( "cannot wrap unbound method" );
    PyObjectPtr pywrapper;
    if( CAtom::TypeCheck( PyMethod_GET_SELF( method ) ) )
    {
        pywrapper = PyType_GenericNew( &AtomMethodWrapper_Type, 0, 0 );
        if( !pywrapper )
            return 0;
        AtomMethodWrapper* wrapper = reinterpret_cast<AtomMethodWrapper*>( pywrapper.get() );
        wrapper->im_func = newref( PyMethod_GET_FUNCTION( method ) );
        // placement new since Python malloc'd and zero'd the struct
        new( &wrapper->pointer ) CAtomPointer( catom_cast( PyMethod_GET_SELF( method ) ) );
    }
    else
    {
        PyObjectPtr wr( PyWeakref_NewRef( PyMethod_GET_SELF( method ), 0 ) );
        if( !wr )
            return 0;
        pywrapper = PyType_GenericNew( &MethodWrapper_Type, 0, 0 );
        if( !pywrapper )
            return 0;
        MethodWrapper* wrapper = reinterpret_cast<MethodWrapper*>( pywrapper.get() );
        wrapper->im_func = newref( PyMethod_GET_FUNCTION( method ) );
        wrapper->im_selfref = wr.release();
    }
    return pywrapper.release();
}


int import_methodwrapper()
{
    if( PyType_Ready( &MethodWrapper_Type ) < 0 )
        return -1;
    return 0;
}
