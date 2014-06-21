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

// POD struct - all member fields are considered private
struct Validator
{
    typedef PyObject* ( *Handler )(
        Validator* validator, PyObject* atom, PyObject* name, PyObject* value
    );

    PyObject_HEAD
    PyObject* m_context;
    PyObject* m_error_handler;
    Handler m_validate_handler;

    enum Mode
    {
        Bool,
        Int,
        Float,
        Bytes,
        Str,
        Unicode,
        Tuple,
        List,
        Dict,
        Instance,
        Typed,
        Subclass,
        Enum,
        Callable,
        Range,
        Coerced,
        Last // sentinel
    };

    static PyTypeObject TypeObject;

    static bool Ready();

    static bool TypeCheck( PyObject* ob )
    {
        return Py_TYPE( ob ) == &TypeObject;
    }
};

} // namespace atom
