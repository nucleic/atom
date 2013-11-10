/*-----------------------------------------------------------------------------
| Copyright (c) 2013, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once
#include <Python.h>

namespace utils
{

inline bool
basestring_check( PyObject* obj )
{
    return (
        PyString_CheckExact( obj ) ||
        PyUnicode_CheckExact( obj ) ||
        PyObject_TypeCheck( obj, &PyBaseString_Type )
    );
}

} // namespace utils
