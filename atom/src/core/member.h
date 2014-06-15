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

// POD struct - all member fields are considered private
struct Member
{
    PyObject_HEAD
    PyObject* m_default;
    PyObject* m_validate;
    PyObject* m_post_validate;
    PyObject* m_post_setattr;
    uint32_t m_flags;

    enum Flag
    {
    };

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

    PyObject* getDefault( PyObject* atom, PyObject* name );

    PyObject* validate( PyObject* atom, PyObject* name, PyObject* value );

    int postSetAttr( PyObject* atom, PyObject* name, PyObject* value );
};

} // namespace atom
