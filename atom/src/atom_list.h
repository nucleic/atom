/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once

#include <Python.h>


namespace atom
{

struct Member;


// POD struct - all member fields are considered private
struct AtomList
{
    PyListObject m_list;
    PyObject* m_atomref;
    PyObject* m_name;
    Member* m_member;

    static PyTypeObject TypeObject;

    static bool Ready();

    static bool TypeCheck( PyObject* ob )
    {
        return PyObject_TypeCheck( ob, &TypeObject ) != 0;
    }

    static PyObject* Create(
    	Member* member, PyObject* atom, PyObject* name, PyObject* items );
};

}  // namespace atom
