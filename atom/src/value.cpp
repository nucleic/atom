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
#include "value.h"

#include "ignoredwarnings.h"


using namespace PythonHelpers;


static int
Value_init( Value* self, PyObject* args, PyObject* kwargs )
{
    static char* kwlist[] = { "default", "factory", 0 };
    PyObject* defaultv = 0;
    PyObject* factory = 0;
    if( !PyArg_ParseTupleAndKeywords( args, kwargs, "|OO:__new__", kwlist, &defaultv, &factory ) )
    {
        return 0;
    }
    if( factory )
    {
    	if( !PyCallable_Check( factory ) )
    	{
    		py_expected_type_fail( factory, "callable" );
	    	return -1;
    	}
    	self->default_context = newref( factory );
    	Member_SetFlag( &self->base, Member::DefaultFactoryContext );
        Member_SetFlag( &self->base, Member::MemberDefault );
    }
    else if( defaultv )
    {
    	self->default_context = newref( defaultv );
    }
    return 0;
}


static void
Value_clear( Value* self )
{
    Py_CLEAR( self->default_context );
    Value_Type.tp_base->tp_clear( reinterpret_cast<PyObject*>( self ) );
}


static int
Value_traverse( Value* self, visitproc visit, void* arg )
{
    Py_VISIT( self->default_context );
    return Value_Type.tp_base->tp_traverse( reinterpret_cast<PyObject*>( self ), visit, arg );
}


static PyObject*
Value_default( Value* self, PyObject* args )
{
	if( !self->default_context )
	{
		Py_RETURN_NONE;
	}
	if( Member_TestFlag( &self->base, Member::DefaultFactoryContext ) )
	{
		PyObjectPtr py_args( PyTuple_New( 0 ) );
		if( !py_args )
		{
			return 0;
		}
		return PyObject_Call( self->default_context, py_args.get(), 0 );
	}
	return newref( self->default_context );
}


static PyObject*
Value_get_default_value( Value* self, void* ctxt )
{
	if( !self->default_context )
	{
		Py_RETURN_NONE;
	}
	if( Member_TestFlag( &self->base, Member::DefaultFactoryContext ) )
	{
		Py_RETURN_NONE;
	}
	return newref( self->default_context );
}


static int
Value_set_default_value( Value* self, PyObject* value, void* ctxt )
{
	/*
	PyObject* old = self->default_value;
	self->default_value = value;
	Py_XINCREF( value );
	Py_XDECREF( old );
	*/
	return 0;
}


static PyObject*
Value_get_default_factory( Value* self, void* ctxt )
{
	if( !self->default_context )
	{
		Py_RETURN_NONE;
	}
	if( Member_TestFlag( &self->base, Member::DefaultFactoryContext ) )
	{
		return newref( self->default_context );
	}
	Py_RETURN_NONE;
}


static int
Value_set_default_factory( Value* self, PyObject* value, void* ctxt )
{
	/*
	if( value && !PyCallable_Check( value ) )
	{
		py_expected_type_fail( value, "callable" );
		return -1;
	}
	PyObject* old = self->default_factory;
	self->default_factory = value;
	Py_XINCREF( value );
	Py_XDECREF( old );
	*/
	return 0;
}


static PyMethodDef
Value_methods[] = {
    { "default", ( PyCFunction )Value_default, METH_VARARGS, "compute the default value" },
    { 0 }  /* sentinel */
};


static PyGetSetDef
Value_getset[] = {
	{ "default_value",
	  ( getter )Value_get_default_value,
	  ( setter )Value_set_default_value,
	  "Get and set the scalar default value for the Value member." },
	{ "default_factory",
 	  ( getter )Value_get_default_factory,
 	  ( setter )Value_set_default_factory,
 	  "Get and set the default factory for the value Value member."},
 	{ 0 }  // sentinel
};


PyTypeObject Value_Type = {
    PyObject_HEAD_INIT( &PyType_Type )
    0,                                      /* ob_size */
    "atom.catom.Value",                     /* tp_name */
    sizeof( Value ),                        /* tp_basicsize */
    0,                                      /* tp_itemsize */
    (destructor)0,			                /* tp_dealloc */
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
    (traverseproc)Value_traverse,           /* tp_traverse */
    (inquiry)Value_clear,                   /* tp_clear */
    (richcmpfunc)0,                         /* tp_richcompare */
    0,                                      /* tp_weaklistoffset */
    (getiterfunc)0,                         /* tp_iter */
    (iternextfunc)0,                        /* tp_iternext */
    (struct PyMethodDef*)Value_methods,     /* tp_methods */
    (struct PyMemberDef*)0,                 /* tp_members */
    Value_getset,                           /* tp_getset */
    &Member_Type,                           /* tp_base */
    0,                                      /* tp_dict */
    (descrgetfunc)0,                        /* tp_descr_get */
    (descrsetfunc)0,                        /* tp_descr_set */
    0,                                      /* tp_dictoffset */
    (initproc)Value_init,                   /* tp_init */
    (allocfunc)PyType_GenericAlloc,         /* tp_alloc */
    (newfunc)0,			                    /* tp_new */
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
import_value()
{
    if( PyType_Ready( &Value_Type ) < 0 )
    {
        return -1;
    }
    return 0;
}
