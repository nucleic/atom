/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once

#include <utils/py23compat.h>
#include <utils/stdint.h>

#include <Python.h>


namespace atom
{

struct ClassMapEntry;
struct Member;


// POD struct - all member fields are considered private
struct ClassMap
{
    PyObject_HEAD
    ClassMapEntry* m_entries;
    uint32_t m_allocated;
    uint32_t m_count;

    static PyTypeObject TypeObject;

    static bool Ready();

    static bool TypeCheck( PyObject* ob )
    {
        return Py_TYPE( ob ) == &TypeObject;
    }

    uint32_t memberCount()
    {
        return m_count;
    }

    // Borrowed member + index on success, untouched on non-existent
    void member( Py23StrObject* name, Member** member, uint32_t* index );
};

} // namespace atom
