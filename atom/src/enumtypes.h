/*-----------------------------------------------------------------------------
| Copyright (c) 2013, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once
#include "pythonhelpers.h"
#include "behaviors.h"


extern PyObject* PyGetAttr;
extern PyObject* PySetAttr;
extern PyObject* PyDelAttr;
extern PyObject* PyPostGetAttr;
extern PyObject* PyPostSetAttr;
extern PyObject* PyDefaultValue;
extern PyObject* PyValidate;
extern PyObject* PyPostValidate;


int import_enumtypes();


namespace EnumTypes
{

template<typename T> inline bool
_convert( PyObject* value, PyObject* py_type, T& out )
{
    PyTypeObject* ob_type = reinterpret_cast<PyTypeObject*>( py_type );
    if( !PyObject_TypeCheck( value, ob_type ) )
    {
        PythonHelpers::py_expected_type_fail( value, ob_type->tp_name );
        return false;
    }
    long lval = PyInt_AsLong( value );
    if( lval == -1 && PyErr_Occurred() )
        return false;
    out = static_cast<T>( lval );
    return true;
}


template<typename T> inline bool
convert( PyObject* value, T& out );


template<> inline bool
convert( PyObject* value, GetAttr::Mode& out )
{
    return _convert( value, PyGetAttr, out );
}


template<> inline bool
convert( PyObject* value, SetAttr::Mode& out )
{
    return _convert( value, PySetAttr, out );
}


template<> inline bool
convert( PyObject* value, DelAttr::Mode& out )
{
    return _convert( value, PyDelAttr, out );
}


template<> inline bool
convert( PyObject* value, PostGetAttr::Mode& out )
{
    return _convert( value, PyPostGetAttr, out );
}


template<> inline bool
convert( PyObject* value, PostSetAttr::Mode& out )
{
    return _convert( value, PyPostSetAttr, out );
}


template<> inline bool
convert( PyObject* value, DefaultValue::Mode& out )
{
    return _convert( value, PyDefaultValue, out );
}


template<> inline bool
convert( PyObject* value, Validate::Mode& out )
{
    return _convert( value, PyValidate, out );
}


template<> inline bool
convert( PyObject* value, PostValidate::Mode& out )
{
    return _convert( value, PyPostValidate, out );
}

}  // namespace EnumTypes
