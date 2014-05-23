/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once

#include <Python.h>
#include "inttypes.h"


struct Member;


int
import_descriptor();


extern PyTypeObject Descriptor_Type;


struct Descriptor
{
	typedef uint16_t flags_t;

	enum Flag
	{
		IsOwned = 0x1,
	};

	PyObject_HEAD
	PyObject* static_members;		// PyDictObject - never null
	PyObject* instance_members; 	// PyDictObject - maybe null
	uint16_t slot_count;
	flags_t flags;
};


inline int
Descriptor_Check( PyObject* object )
{
    return PyObject_TypeCheck( object, &Descriptor_Type );
}


Member* 	// borrowed reference on success, null on not-found
Descriptor_LookupMember( Descriptor* descr, PyStringObject* name );


bool
Descriptor_AddStaticMember( Descriptor* descr, PyStringObject* name, Member* member );


bool
Descriptor_AddInstanceMember( Descriptor* descr, PyStringObject* name, Member* member );
