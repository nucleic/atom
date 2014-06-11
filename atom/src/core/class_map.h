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


namespace atom
{

struct ClassMapEntry;


// POD struct - all member fields are considered private
struct ClassMap
{
    PyObject_HEAD
    ClassMapEntry* m_entries;
    uint32_t m_allocated;
    uint32_t m_count;

    static PyTypeObject TypeObject;

    static bool Import();

    static bool TypeCheck( PyObject* ob )
    {
        return Py_TYPE( ob ) == &TypeObject;
    }

    uint32_t getMemberCount()
    {
        return m_count;
    }

    // Borrowed member + index on success, untouched on failure
    void getMember( PyStringObject* name, Member** member, uint32_t* index );
};

} // namespace atom
