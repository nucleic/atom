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


int
import_class_map();


extern PyTypeObject ClassMap_Type;


struct Member;


struct ClassMapEntry;


struct ClassMap
{
	// all fields are considered private
	PyObject_HEAD
	ClassMapEntry* entries;
	uint32_t allocated;
	uint32_t member_count;
};


inline int
ClassMap_Check( PyObject* op )
{
    return Py_TYPE( op ) == &ClassMap_Type;
}


inline uint32_t  // member count will never change after descriptor is created
ClassMap_MemberCount( ClassMap* map )
{
	return map->member_count;
}


void  // borrowed member + index on success, untouched on failure
ClassMap_LookupMember( ClassMap* map, PyStringObject* name, Member** member, uint32_t* index );
