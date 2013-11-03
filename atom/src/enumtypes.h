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
_from_py_enum( PyObject* value, PyObject* py_type, T& out )
{
    PyTypeObject* ob_type = reinterpret_cast<PyTypeObject*>( py_type );
    if( !PyObject_TypeCheck( value, ob_type ) )
    {
        PythonHelpers::py_expected_type_fail( value, ob_type->tp_name );
        return false;
    }
    long lval = PyLong_AsLong( value );
    if( lval == -1 && PyErr_Occurred() )
        return false;
    out = static_cast<T>( lval );
    return true;
}


template<typename T> inline bool
from_py_enum( PyObject* value, T& out );


template<> inline bool
from_py_enum( PyObject* value, GetAttr::Mode& out )
{
    return _from_py_enum( value, PyGetAttr, out );
}


template<> inline bool
from_py_enum( PyObject* value, SetAttr::Mode& out )
{
    return _from_py_enum( value, PySetAttr, out );
}


template<> inline bool
from_py_enum( PyObject* value, DelAttr::Mode& out )
{
    return _from_py_enum( value, PyDelAttr, out );
}


template<> inline bool
from_py_enum( PyObject* value, PostGetAttr::Mode& out )
{
    return _from_py_enum( value, PyPostGetAttr, out );
}


template<> inline bool
from_py_enum( PyObject* value, PostSetAttr::Mode& out )
{
    return _from_py_enum( value, PyPostSetAttr, out );
}


template<> inline bool
from_py_enum( PyObject* value, DefaultValue::Mode& out )
{
    return _from_py_enum( value, PyDefaultValue, out );
}


template<> inline bool
from_py_enum( PyObject* value, Validate::Mode& out )
{
    return _from_py_enum( value, PyValidate, out );
}


template<> inline bool
from_py_enum( PyObject* value, PostValidate::Mode& out )
{
    return _from_py_enum( value, PyPostValidate, out );
}


template<typename T> inline PyObject*
_to_py_enum( T value, PyObject* py_enum_class )
{
    PythonHelpers::PyObjectPtr py_int( PyLong_FromLong( static_cast<long>( value ) ) );
    if( !py_int )
        return 0;
    PythonHelpers::PyTuplePtr py_args( PyTuple_New( 1 ) );
    if( !py_args )
        return 0;
    py_args.set_item( 0, py_int );
    return PyObject_Call( py_enum_class, py_args.get(), 0 );
}


template<typename T> inline PyObject*
to_py_enum( T value );


template<> inline PyObject*
to_py_enum( GetAttr::Mode value )
{
    return _to_py_enum( value, PyGetAttr );
}


template<> inline PyObject*
to_py_enum( SetAttr::Mode value )
{
    return _to_py_enum( value, PySetAttr );
}


template<> inline PyObject*
to_py_enum( DelAttr::Mode value )
{
    return _to_py_enum( value, PyDelAttr );
}


template<> inline PyObject*
to_py_enum( PostGetAttr::Mode value )
{
    return _to_py_enum( value, PyPostGetAttr );
}


template<> inline PyObject*
to_py_enum( PostSetAttr::Mode value )
{
    return _to_py_enum( value, PyPostSetAttr );
}


template<> inline PyObject*
to_py_enum( DefaultValue::Mode value )
{
    return _to_py_enum( value, PyDefaultValue );
}


template<> inline PyObject*
to_py_enum( Validate::Mode value )
{
    return _to_py_enum( value, PyValidate );
}


template<> inline PyObject*
to_py_enum( PostValidate::Mode value )
{
    return _to_py_enum( value, PyPostValidate );
}

}  // namespace EnumTypes
