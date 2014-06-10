/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once

#include <Python.h>
#include <utils/stdint.h>


struct Member;


struct ClassMapEntry
{
    PyStringObject* name;
    Member* member;
    uint32_t index;
};


struct ClassMap
{
    PyObject_HEAD;
    ClassMapEntry* entries;
    uint32_t allocated;
    uint32_t count;
};


// map count is a constant
inline uint32_t ClassMap_GetCount( ClassMap* map )
{
    return map->count;
}


extern PyTypeObject ClassMap_Type;


inline int ClassMap_Check( PyObject* ob )
{
    return Py_TYPE( ob ) == &ClassMap_Type;
}


// borrowed member + index on success, untouched on failure
void ClassMap_LookupMember( ClassMap* map,
                            PyStringObject* name,
                            Member** member,
                            uint32_t* index );


int import_class_map();
