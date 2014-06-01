/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include <Python.h>
#include "pythonhelpers.h"
#include "default_value_behavior.h"
#include "member.h"


using namespace PythonHelpers;


bool DefaultValue_CheckMode( DefaultValue::Mode mode, PyObject* context )
{
    switch( mode )
    {
    case DefaultValue::List:
        if( context != Py_None && !PyList_Check( context ) )
        {
            py_expected_type_fail( context, "list or None" );
            return false;
        }
        break;
    case DefaultValue::Dict:
        if( context != Py_None && !PyDict_Check( context ) )
        {
            py_expected_type_fail( context, "dict or None" );
            return false;
        }
        break;
    case DefaultValue::CallObject:
        if( !PyCallable_Check( context ) )
        {
            py_expected_type_fail( context, "callable" );
            return false;
        }
        break;
    case DefaultValue::ObjectMethod:
    case DefaultValue::MemberMethod:
        if( !PyString_Check( context ) )
        {
            py_expected_type_fail( context, "str" );
            return false;
        }
        break;
    default:
        break;
    }
    return true;
}


static PyObject*
no_op_handler( Member* member, CAtom* atom, PyStringObject* name )
{
    return newref( Py_None );
}


static PyObject*
static_handler( Member* member, CAtom* atom, PyStringObject* name )
{
    return newref( member->default_value_context );
}


static PyObject*
list_handler( Member* member, CAtom* atom, PyStringObject* name )
{
    if( member->default_value_context == Py_None )
    {
        return PyList_New( 0 );
    }
    Py_ssize_t size = PyList_GET_SIZE( member->default_value_context );
    return PyList_GetSlice( member->default_value_context, 0, size );
}


static PyObject*
dict_handler( Member* member, CAtom* atom, PyStringObject* name )
{
    if( member->default_value_context == Py_None )
    {
        return PyDict_New();
    }
    return PyDict_Copy( member->default_value_context );
}


static PyObject*
call_object_handler( Member* member, CAtom* atom, PyStringObject* name )
{
    PyTuplePtr args( PyTuple_New( 0 ) );
    if( !args )
    {
        return 0;
    }
    return PyObject_Call( member->default_value_context, args.get(), 0 );
}


static PyObject*
object_method_handler( Member* member, CAtom* atom, PyStringObject* name )
{
    PyObjectPtr callable(
        PyObject_GetAttr( ( PyObject* )atom, member->default_value_context ) );
    if( !callable )
    {
        return 0;
    }
    PyTuplePtr args( PyTuple_New( 0 ) );
    if( !args )
    {
        return 0;
    }
    return callable( args ).release();
}


static PyObject*
member_method_handler( Member* member, CAtom* atom, PyStringObject* name )
{
    PyObjectPtr callable( PyObject_GetAttr( ( PyObject* )member,
                                            member->default_value_context ) );
    if( !callable )
    {
        return 0;
    }
    PyTuplePtr args( PyTuple_New( 2 ) );
    if( !args )
    {
        return 0;
    }
    args.initialize( 0, newref( ( PyObject* )atom ) );
    args.initialize( 1, newref( ( PyObject* )name ) );
    return callable( args ).release();
}


typedef PyObject* ( *DefaultValueHandler )( Member* member,
                                            CAtom* atom,
                                            PyStringObject* name );


static DefaultValueHandler dfv_handlers[] = {no_op_handler,
                                             static_handler,
                                             list_handler,
                                             dict_handler,
                                             call_object_handler,
                                             object_method_handler,
                                             member_method_handler};


PyObject*
Member_DefaultValue( Member* member, CAtom* atom, PyStringObject* name )
{
    return dfv_handlers[member->default_value_mode]( member, atom, name );
}
