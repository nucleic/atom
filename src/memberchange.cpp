/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic
|
| Distributed under the terms of the BSD 3-Clause License.
|
| The full license is in the file LICENSE, distributed with this software.
|----------------------------------------------------------------------------*/
#include "memberchange.h"

#include <cppy/cppy.h>


#define FREELIST_MAX 64

#define memberchange_cast( o ) reinterpret_cast<MemberChange*>( o )
#define pyobject_cast( o ) reinterpret_cast<PyObject*>( o )


namespace atom
{

namespace
{

int numfree = 0;
MemberChange* freelist[ FREELIST_MAX ];


int MemberChange_clear( MemberChange* self )
{
	Py_CLEAR( self->m_object );
	Py_CLEAR( self->m_name );
	Py_CLEAR( self->m_old_value );
	Py_CLEAR( self->m_new_value );
	return 0;
}


int MemberChange_traverse( MemberChange* self, visitproc visit, void* arg )
{
	Py_VISIT( self->m_object );
	Py_VISIT( self->m_name );
	Py_VISIT( self->m_old_value );
	Py_VISIT( self->m_new_value );
	return 0;
}


void MemberChange_dealloc( MemberChange* self )
{
	PyObject_GC_UnTrack( self );
	MemberChange_clear( self );
	if( numfree < FREELIST_MAX )
	{
		freelist[ numfree++ ] = self;
	}
	else
	{
		Py_TYPE( self )->tp_free( pyobject_cast( self ) );
	}
}


PyObject* MemberChange_get_object( MemberChange* self, void* ctxt )
{
	return cppy::incref( self->m_object );
}


PyObject* MemberChange_get_name( MemberChange* self, void* ctxt )
{
	return cppy::incref( self->m_name );
}


PyObject* MemberChange_get_old_value( MemberChange* self, void* ctxt )
{
	return cppy::incref( self->m_old_value );
}


PyObject* MemberChange_get_new_value( MemberChange* self, void* ctxt )
{
	return cppy::incref( self->m_new_value );
}


PyGetSetDef MemberChange_getset[] = {
	{ "object",
		( getter )MemberChange_get_object, 0,
		"the object which changed", 0 },
	{ "name",
		( getter )MemberChange_get_name, 0,
		"the name of the changed member", 0 },
	{ "old_value",
		( getter )MemberChange_get_old_value, 0,
		"the old value of the member", 0 },
	{ "new_value",
		( getter )MemberChange_get_new_value, 0,
		"the new value of the member", 0 },
	{ 0 } // sentinel
};

} // namespace


PyTypeObject MemberChange::TypeObject = {
	PyVarObject_HEAD_INIT( &PyType_Type, 0 )
	"atom.catom.MemberChange",                   /* tp_name */
	sizeof( MemberChange ),                      /* tp_basicsize */
	0,                                           /* tp_itemsize */
	( destructor )MemberChange_dealloc,          /* tp_dealloc */
	( printfunc )0,                              /* tp_print */
	( getattrfunc )0,                            /* tp_getattr */
	( setattrfunc )0,                            /* tp_setattr */
#ifdef IS_PY3K
	( void* )0,                                  /* tp_reserved */
#else
	( cmpfunc )0,                                /* tp_compare */
#endif
	( reprfunc )0,                               /* tp_repr */
	( PyNumberMethods* )0,                       /* tp_as_number */
	( PySequenceMethods* )0,                     /* tp_as_sequence */
	( PyMappingMethods* )0,                      /* tp_as_mapping */
	( hashfunc )0,                               /* tp_hash */
	( ternaryfunc )0,                            /* tp_call */
	( reprfunc )0,                               /* tp_str */
	( getattrofunc )0,                           /* tp_getattro */
	( setattrofunc )0,                           /* tp_setattro */
	( PyBufferProcs* )0,                         /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC,     /* tp_flags */
	0,                                           /* Documentation string */
	( traverseproc )MemberChange_traverse,       /* tp_traverse */
	( inquiry )MemberChange_clear,               /* tp_clear */
	( richcmpfunc )0,                            /* tp_richcompare */
	0,                                           /* tp_weaklistoffset */
	( getiterfunc )0,                            /* tp_iter */
	( iternextfunc )0,                           /* tp_iternext */
	( struct PyMethodDef* )0,                    /* tp_methods */
	( struct PyMemberDef* )0,                    /* tp_members */
	MemberChange_getset,                         /* tp_getset */
	0,                                           /* tp_base */
	0,                                           /* tp_dict */
	( descrgetfunc )0,                           /* tp_descr_get */
	( descrsetfunc )0,                           /* tp_descr_set */
	0,                                           /* tp_dictoffset */
	( initproc )0,                               /* tp_init */
	( allocfunc )PyType_GenericAlloc,            /* tp_alloc */
	( newfunc )0,                                /* tp_new */
	( freefunc )PyObject_GC_Del,                 /* tp_free */
	( inquiry )0,                                /* tp_is_gc */
	0,                                           /* tp_bases */
	0,                                           /* tp_mro */
	0,                                           /* tp_cache */
	0,                                           /* tp_subclasses */
	0,                                           /* tp_weaklist */
	( destructor )0                              /* tp_del */
};


bool MemberChange::Ready()
{
	return PyType_Ready( &TypeObject ) == 0;
}


PyObject* MemberChange::Create( PyObject* object, PyObject* name, PyObject* old_value, PyObject* new_value )
{
	PyObject* pyo;
	if( numfree > 0 )
	{
		pyo = pyobject_cast( freelist[ --numfree ] );
		_Py_NewReference( pyo );
		PyObject_GC_Track( pyo );
	}
	else
	{
		pyo = PyType_GenericAlloc( &TypeObject, 0 );
		if( !pyo )
		{
			return 0;
		}
	}
	MemberChange* change = memberchange_cast( pyo );
	change->m_object = cppy::incref( object );
	change->m_name = cppy::incref( name );
	change->m_old_value = cppy::incref( old_value );
	change->m_new_value = cppy::incref( new_value );
	return pyo;
}

} // namespace atom
