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
struct ClassMap;


extern PyTypeObject ClassMap_Type;


inline int ClassMap_Check( PyObject* ob )
{
    return Py_TYPE( ob ) == &ClassMap_Type;
}


// count number of members and is a constant
uint32_t ClassMap_Count( ClassMap* map );


// borrowed member + index on success, untouched on failure
void ClassMap_Lookup( ClassMap* map,
                      PyStringObject* name,
                      Member** member,
                      uint32_t* index );


int import_class_map();
