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

struct ClassMap;


// POD struct - all member fields are considered private
struct Atom
{
    PyObject_HEAD
    ClassMap* m_class_map;
    PyObject** m_slots;
    uint32_t m_flags;

    enum Flag
    {};

    static PyTypeObject TypeObject;

    static bool Ready();

    static bool TypeCheck( PyObject* ob )
    {
        return PyObject_TypeCheck( ob, &TypeObject ) != 0;
    }

    bool testFlag( Flag flag )
    {
        return ( m_flags & static_cast<uint32_t>( flag ) ) != 0;
    }

    bool setFlag( Flag flag, bool on = true )
    {
        if( on )
        {
            m_flags |= static_cast<uint32_t>( flag );
        }
        else
        {
            m_flags &= ~( static_cast<uint32_t>( flag ) );
        }
    }
};

}  // namespace atom
