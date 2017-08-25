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

#if PY_MAJOR_VERSION >= 3
    #define STR_TYPE PyUnicode_Type
    #define STR_CHECK_EXACT( obj ) PyUnicode_CheckExact( obj )
#else
    #define STR_TYPE PyBaseString_Type
    #define STR_CHECK_EXACT( obj ) PyString_CheckExact( obj ) || PyUnicode_CheckExact( obj )
#endif


inline bool
basestring_check( PyObject* obj )
{
    return (
        STR_CHECK_EXACT( obj ) ||
        PyObject_TypeCheck( obj, &STR_TYPE )
    );
}

} // namespace utils
