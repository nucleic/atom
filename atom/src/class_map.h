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


int import_class_map();


extern PyTypeObject ClassMap_Type;


struct Member;


struct ClassMapEntry;


struct ClassMap
{
    PyObject_HEAD;
    ClassMapEntry* entries;
    uint32_t allocated;
    uint32_t member_count;
};


inline int ClassMap_Check( PyObject* op )
{
    return Py_TYPE( op ) == &ClassMap_Type;
}


// member count will never change after descriptor is created
inline uint32_t ClassMap_MemberCount( ClassMap* map )
{
    return map->member_count;
}


// borrowed member + index on success, untouched on failure
void ClassMap_LookupMember( ClassMap* map,
                            PyStringObject* name,
                            Member** member,
                            uint32_t* index );
