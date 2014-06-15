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

namespace Validate
{

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

} // namespace Validate


struct Validator;


typedef PyObject* ( *ValidateHandler )(
    Validator* validator, PyObject* atom, PyObject* name, PyObject* value );


// POD struct - all member fields are considered private
struct Validator
{
    PyObject_HEAD
    PyObject* m_context;
    ValidateHandler m_handler;

    static PyTypeObject TypeObject;

    static bool Ready();

    static bool TypeCheck( PyObject* ob )
    {
        return Py_TYPE( ob ) == &TypeObject;
    }
};

} // namespace atom
