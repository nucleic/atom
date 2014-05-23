/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include <algorithm>
#include <Python.h>
#include "pythonhelpers.h"
#include "inttypes.h"
#include "descriptor.h"
#include "member.h"

#include "ignoredwarnings.h"


using namespace PythonHelpers;


static uint16_t
compute_slot_index( Descriptor* descr, PyStringObject* name )
{
	PyObject* py_name = reinterpret_cast<PyObject*>( name );
	PyObject* py_member = PyDict_GetItem( descr->static_members, py_name );
	if( py_member )
	{
		return reinterpret_cast<Member*>( py_member )->slot_index;
	}
	if( descr->instance_members )
	{
		py_member = PyDict_GetItem( descr->instance_members, py_name );
		if( py_member )
		{
			return reinterpret_cast<Member*>( py_member )->slot_index;
		}
	}
	return descr->slot_count;
}


Member*
Descriptor_LookupMember( Descriptor* descr, PyStringObject* name )
{
	PyObject* py_member;
	PyObject* py_name = reinterpret_cast<PyObject*>( name );
	if( descr->instance_members )
	{
		py_member = PyDict_GetItem( descr->instance_members, py_name );
		if( py_member )
		{
			return reinterpret_cast<Member*>( py_member );
		}
	}
	py_member = PyDict_GetItem( descr->static_members, py_name );
	if( py_member )
	{
		return reinterpret_cast<Member*>( py_member );
	}
	return 0;
}


bool
Descriptor_AddStaticMember( Descriptor* descr, PyStringObject* name, Member* member )
{
	// XXX ensure unowned member then set to owned
	PyObject* py_name = reinterpret_cast<PyObject*>( name );
	PyObject* py_member = reinterpret_cast<PyObject*>( member );
	uint16_t slot_index = compute_slot_index( descr, name );
	bool ok = PyDict_SetItem( descr->static_members, py_name, py_member ) == 0;
	if( ok )
	{
		member->slot_index = slot_index;
		descr->slot_count = std::max<uint16_t>( descr->slot_count, slot_index + 1 );
	}
	return ok;
}


bool
Descriptor_AddInstanceMember( Descriptor* descr, PyStringObject* name, Member* member )
{
	// XXX ensure unowned member then set to owned
	PyObject* py_name = reinterpret_cast<PyObject*>( name );
	PyObject* py_member = reinterpret_cast<PyObject*>( member );
	uint16_t slot_index = compute_slot_index( descr, name );
	if( !descr->instance_members )
	{
		descr->instance_members = PyDict_New();
		if( !descr->instance_members )
		{
			return false;
		}
	}
	bool ok = PyDict_SetItem( descr->instance_members, py_name, py_member ) == 0;
	if( ok )
	{
		member->slot_index = slot_index;
		descr->slot_count = std::max<uint16_t>( descr->slot_count, slot_index + 1 );
	}
	return ok;
}


static PyObject*
Descriptor_new( PyTypeObject* type, PyObject* args, PyObject* kwargs )
{
	static char *kwlist[] = { "members", 0 };
    PyObject* members;
    if( !PyArg_ParseTupleAndKeywords( args, kwargs, "O:__new__", kwlist, &members ) )
    {
        return 0;
    }
    if( !PyDict_CheckExact( members ) )
    {
    	return py_expected_type_fail( members, "dict" );
    }
	PyObjectPtr self_ptr( PyType_GenericNew( type, 0, 0 ) );
	if( !self_ptr )
	{
		return 0;
	}
	Descriptor* descr = reinterpret_cast<Descriptor*>( self_ptr.get() );
	descr->static_members = PyDict_New();
	if( !descr->static_members )
	{
		return 0;
	}
    PyObject* key;
    PyObject* value;
    Py_ssize_t pos = 0;
    PyStringObject* py_str;
    Member* py_mbr;
    while( PyDict_Next( members, &pos, &key, &value ) )
    {
    	if( !PyString_Check( key ) )
    	{
    		return py_expected_type_fail( key, "str" );
    	}
    	if( !Member_Check( value ) )
    	{
    		return py_expected_type_fail( value, "Member" );
    	}
    	py_str = reinterpret_cast<PyStringObject*>( key );
    	py_mbr = reinterpret_cast<Member*>( value );
    	if( !Descriptor_AddStaticMember( descr, py_str, py_mbr ) )
    	{
    		return 0;
    	}
    }
	return self_ptr.release();
}


static void
Descriptor_clear( Descriptor* self )
{
	Py_CLEAR( self->static_members );
	Py_CLEAR( self->instance_members );
}


static int
Descriptor_traverse( Descriptor* self, visitproc visit, void* arg )
{
	Py_VISIT( self->static_members );
	Py_VISIT( self->instance_members );
    return 0;
}


static void
Descriptor_dealloc( Descriptor* self )
{
    PyObject_GC_UnTrack( self );
    Descriptor_clear( self );
    self->ob_type->tp_free( reinterpret_cast<PyObject*>( self ) );
}


PyTypeObject Descriptor_Type = {
    PyObject_HEAD_INIT( &PyType_Type )
    0,                                      /* ob_size */
    "atom.catom.Descriptor",                /* tp_name */
    sizeof( Descriptor ),                   /* tp_basicsize */
    0,                                      /* tp_itemsize */
    (destructor)Descriptor_dealloc,         /* tp_dealloc */
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
    Py_TPFLAGS_DEFAULT|Py_TPFLAGS_HAVE_GC,  /* tp_flags */
    0,                                      /* Documentation string */
    (traverseproc)Descriptor_traverse,      /* tp_traverse */
    (inquiry)Descriptor_clear,              /* tp_clear */
    (richcmpfunc)0,                         /* tp_richcompare */
    0,                                      /* tp_weaklistoffset */
    (getiterfunc)0,                         /* tp_iter */
    (iternextfunc)0,                        /* tp_iternext */
    (struct PyMethodDef*)0,				    /* tp_methods */
    (struct PyMemberDef*)0,                 /* tp_members */
    0,			                            /* tp_getset */
    0,                                      /* tp_base */
    0,                                      /* tp_dict */
    (descrgetfunc)0,                        /* tp_descr_get */
    (descrsetfunc)0,                        /* tp_descr_set */
    0,                                      /* tp_dictoffset */
    (initproc)0,                            /* tp_init */
    (allocfunc)PyType_GenericAlloc,         /* tp_alloc */
    (newfunc)Descriptor_new,                /* tp_new */
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
import_descriptor()
{
    if( PyType_Ready( &Descriptor_Type ) < 0 )
    {
        return -1;
    }
    return 0;
}
