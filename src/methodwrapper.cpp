/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include "methodwrapper.h"

#include <cppy/cppy.h>


#define pyobject_cast( o ) reinterpret_cast<PyObject*>( o )
#define methodwrapper_cast( o ) reinterpret_cast<MethodWrapper*>( o )


namespace atom
{

namespace
{

int MethodWrapper_clear( MethodWrapper* self )
{
	Py_CLEAR( self->m_func );
	Py_CLEAR( self->m_obref );
	return 0;
}


int MethodWrapper_traverse( MethodWrapper* self, visitproc visit, void* arg )
{
	Py_VISIT( self->m_func );
	Py_VISIT( self->m_obref );
	return 0;
}


void MethodWrapper_dealloc( MethodWrapper* self )
{
	MethodWrapper_clear( self );
	Py_TYPE( self )->tp_free( pyobject_cast( self ) );
}


PyObject* MethodWrapper_call( MethodWrapper* self, PyObject* args, PyObject* kwargs )
{
	PyObject* pyo = PyWeakref_GET_OBJECT( self->m_obref );
	if( pyo == Py_None )
	{
		return cppy::incref( Py_None );
	}
	Py_ssize_t count = PyTuple_GET_SIZE( args );
	cppy::ptr newargs( PyTuple_New( count + 1 ) );
	if( !newargs )
	{
		return 0;
	}
	PyTuple_SET_ITEM( newargs.get(), 0, cppy::incref( pyo ) );
	for( Py_ssize_t i = 0; i < count; ++i )
	{
		PyObject* v = PyTuple_GET_ITEM( args, i );
		PyTuple_SET_ITEM( newargs.get(), i + 1, cppy::incref( v ) );
	}
	return PyObject_Call( self->m_func, newargs.get(), kwargs );
}


PyObject* MethodWrapper_richcompare( MethodWrapper* self, PyObject* rhs, int op )
{
	if( op == Py_EQ || op == Py_NE )
	{
		bool res = false;
		if( MethodWrapper::TypeCheck( rhs ) )
		{
			MethodWrapper* other = methodwrapper_cast( rhs );
			res = self->m_func == other->m_func && self->m_obref == other->m_obref;
		}
		else if ( PyMethod_Check( rhs ) && PyMethod_GET_SELF( rhs ) )
		{
			PyObject* ob1 = PyWeakref_GET_OBJECT( self->m_obref );
			PyObject* ob2 = PyMethod_GET_SELF( rhs );
			PyObject* func = PyMethod_GET_FUNCTION( rhs );
			res = self->m_func == func && ob1 == ob2;
		}
		if( op == Py_NE )
		{
			res = !res;
		}
		return cppy::incref( res ? Py_True : Py_False );
	}
	return cppy::incref( Py_NotImplemented );
}


int MethodWrapper_nonzero( MethodWrapper* self )
{
	return PyWeakref_GET_OBJECT( self->m_obref ) != Py_None;
}


PyNumberMethods MethodWrapper_as_number = {
	( binaryfunc )0,                       /* nb_add */
	( binaryfunc )0,                       /* nb_subtract */
	( binaryfunc )0,                       /* nb_multiply */
#ifndef IS_PY3K
	( binaryfunc )0,                       /* nb_divide */
#endif
	( binaryfunc )0,                       /* nb_remainder */
	( binaryfunc )0,                       /* nb_divmod */
	( ternaryfunc )0,                      /* nb_power */
	( unaryfunc )0,                        /* nb_negative */
	( unaryfunc )0,                        /* nb_positive */
	( unaryfunc )0,                        /* nb_absolute */
	( inquiry )MethodWrapper_nonzero       /* nb_nonzero */
};

} // namespace


PyTypeObject MethodWrapper::TypeObject = {
	PyVarObject_HEAD_INIT( &PyType_Type, 0 )
	"atom.catom.MethodWrapper",                   /* tp_name */
	sizeof( MethodWrapper ),                      /* tp_basicsize */
	0,                                            /* tp_itemsize */
	( destructor )MethodWrapper_dealloc,          /* tp_dealloc */
	( printfunc )0,                               /* tp_print */
	( getattrfunc )0,                             /* tp_getattr */
	( setattrfunc )0,                             /* tp_setattr */
#ifdef IS_PY3K
	( void* )0,                                   /* tp_reserved */
#else
	( cmpfunc )0,                                 /* tp_compare */
#endif
	( reprfunc )0,                                /* tp_repr */
	( PyNumberMethods* )&MethodWrapper_as_number, /* tp_as_number */
	( PySequenceMethods* )0,                      /* tp_as_sequence */
	( PyMappingMethods* )0,                       /* tp_as_mapping */
	( hashfunc )0,                                /* tp_hash */
	( ternaryfunc )MethodWrapper_call,            /* tp_call */
	( reprfunc )0,                                /* tp_str */
	( getattrofunc )0,                            /* tp_getattro */
	( setattrofunc )0,                            /* tp_setattro */
	( PyBufferProcs* )0,                          /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC,      /* tp_flags */
	0,                                            /* Documentation string */
	( traverseproc )MethodWrapper_traverse,       /* tp_traverse */
	( inquiry )MethodWrapper_clear,               /* tp_clear */
	( richcmpfunc )MethodWrapper_richcompare,     /* tp_richcompare */
	0,                                            /* tp_weaklistoffset */
	( getiterfunc )0,                             /* tp_iter */
	( iternextfunc )0,                            /* tp_iternext */
	( struct PyMethodDef* )0,                     /* tp_methods */
	( struct PyMemberDef* )0,                     /* tp_members */
	0,                                            /* tp_getset */
	0,                                            /* tp_base */
	0,                                            /* tp_dict */
	( descrgetfunc )0,                            /* tp_descr_get */
	( descrsetfunc )0,                            /* tp_descr_set */
	0,                                            /* tp_dictoffset */
	( initproc )0,                                /* tp_init */
	( allocfunc )PyType_GenericAlloc,             /* tp_alloc */
	( newfunc )0,                                 /* tp_new */
	( freefunc )PyObject_GC_Del,                  /* tp_free */
	( inquiry )0,                                 /* tp_is_gc */
	0,                                            /* tp_bases */
	0,                                            /* tp_mro */
	0,                                            /* tp_cache */
	0,                                            /* tp_subclasses */
	0,                                            /* tp_weaklist */
	( destructor )0                               /* tp_del */
};


bool MethodWrapper::Ready()
{
	return PyType_Ready( &MethodWrapper::TypeObject ) == 0;
}


PyObject* MethodWrapper::Create( PyMethodObject* method )
{
	if( !PyMethod_GET_SELF( method ) )
	{
		return cppy::type_error( "cannot wrap unbound method" );
	}
	cppy::ptr obref( PyWeakref_NewRef( PyMethod_GET_SELF( method ), 0 ) );
	if( !obref )
	{
		return 0;
	}
	cppy::ptr wrapper = PyType_GenericAlloc( &TypeObject, 0 );
	if( !wrapper )
	{
		return 0;
	}
	MethodWrapper* mw = methodwrapper_cast( wrapper.get() );
	mw->m_func = cppy::incref( PyMethod_GET_FUNCTION( method ) );
	mw->m_obref = obref.release();
	return wrapper.release();
}

} // namespace atom
