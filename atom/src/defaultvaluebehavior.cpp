/*-----------------------------------------------------------------------------
| Copyright (c) 2013, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include "member.h"


using namespace PythonHelpers;


bool
Member::check_context( DefaultValue::Mode mode, PyObject* context )
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
        case DefaultValue::Delegate:
            if( !Member::TypeCheck( context ) )
            {
                py_expected_type_fail( context, "Member" );
                return false;
            }
            break;
        case DefaultValue::CallObject:
        case DefaultValue::CallObject_Object:
        case DefaultValue::CallObject_ObjectName:
            if( !PyCallable_Check( context ) )
            {
                py_expected_type_fail( context, "callable" );
                return false;
            }
            break;
        case DefaultValue::ObjectMethod:
        case DefaultValue::ObjectMethod_Name:
        case DefaultValue::MemberMethod_Object:
            if( !PyUnicode_Check( context ) )
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
no_op_handler( Member* member, CAtom* atom )
{
    return newref( Py_None );
}


static PyObject*
static_handler( Member* member, CAtom* atom )
{
    return newref( member->default_value_context );
}


static PyObject*
list_handler( Member* member, CAtom* atom )
{
    if( member->default_value_context == Py_None )
        return PyList_New( 0 );
    Py_ssize_t size = PyList_GET_SIZE( member->default_value_context );
    return PyList_GetSlice( member->default_value_context, 0, size );
}


static PyObject*
dict_handler( Member* member, CAtom* atom )
{
    if( member->default_value_context == Py_None )
        return PyDict_New();
    return PyDict_Copy( member->default_value_context );
}


static PyObject*
delegate_handler( Member* member, CAtom* atom )
{
    Member* delegate = member_cast( member->default_value_context );
    return delegate->default_value( atom );
}


static PyObject*
call_object_handler( Member* member, CAtom* atom )
{
    PyObjectPtr callable( newref( member->default_value_context ) );
    PyTuplePtr args( PyTuple_New( 0 ) );
    if( !args )
        return 0;
    return callable( args ).release();
}


static PyObject*
call_object_object_handler( Member* member, CAtom* atom )
{
    PyObjectPtr callable( newref( member->default_value_context ) );
    PyTuplePtr args( PyTuple_New( 1 ) );
    if( !args )
        return 0;
    args.initialize( 0, newref( pyobject_cast( atom ) ) );
    return callable( args ).release();
}


static PyObject*
call_object_object_name_handler( Member* member, CAtom* atom )
{
    PyObjectPtr callable( newref( member->default_value_context ) );
    PyTuplePtr args( PyTuple_New( 2 ) );
    if( !args )
        return 0;
    args.initialize( 0, newref( pyobject_cast( atom ) ) );
    args.initialize( 1, newref( member->name ) );
    return callable( args ).release();
}


static PyObject*
object_method_handler( Member* member, CAtom* atom )
{
    PyObjectPtr callable( PyObject_GetAttr( pyobject_cast( atom ), member->default_value_context ) );
    if( !callable )
        return 0;
    PyTuplePtr args( PyTuple_New( 0 ) );
    if( !args )
        return 0;
    return callable( args ).release();
}


static PyObject*
object_method_name_handler( Member* member, CAtom* atom )
{
    PyObjectPtr callable( PyObject_GetAttr( pyobject_cast( atom ), member->default_value_context ) );
    if( !callable )
        return 0;
    PyTuplePtr args( PyTuple_New( 1 ) );
    if( !args )
        return 0;
    args.initialize( 0, newref( member->name ) );
    return callable( args ).release();
}


static PyObject*
member_method_object_handler( Member* member, CAtom* atom )
{
    PyObjectPtr callable( PyObject_GetAttr( pyobject_cast( member ), member->default_value_context ) );
    if( !callable )
        return 0;
    PyTuplePtr args( PyTuple_New( 1 ) );
    if( !args )
        return 0;
    args.initialize( 0, newref( pyobject_cast( atom ) ) );
    return callable( args ).release();
}


typedef PyObject*
( *handler )( Member* member, CAtom* atom );


static handler
handlers[] = {
    no_op_handler,
    static_handler,
    list_handler,
    dict_handler,
    delegate_handler,
    call_object_handler,
    call_object_object_handler,
    call_object_object_name_handler,
    object_method_handler,
    object_method_name_handler,
    member_method_object_handler
};


PyObject*
Member::default_value( CAtom* atom )
{
    if( get_default_value_mode() >= sizeof( handlers ) )
        return no_op_handler( this, atom );
    return handlers[ get_default_value_mode() ]( this, atom );
}
