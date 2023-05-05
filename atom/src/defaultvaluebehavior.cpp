/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2023, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file LICENSE, distributed with this software.
|----------------------------------------------------------------------------*/
#include <cppy/cppy.h>
#include "member.h"


namespace atom
{


bool
Member::check_context( DefaultValue::Mode mode, PyObject* context )
{
    switch( mode )
    {
        case DefaultValue::List:
            if( context != Py_None && !PyList_Check( context ) )
            {
                cppy::type_error( context, "list or None" );
                return false;
            }
            break;
        case DefaultValue::Set:
            if( context != Py_None && !PyAnySet_Check( context ) )
            {
                cppy::type_error( context, "set or None" );
                return false;
            }
            break;
        case DefaultValue::Dict:
        case DefaultValue::DefaultDict:
            if( context != Py_None && !PyDict_Check( context ) )
            {
                cppy::type_error( context, "dict or None" );
                return false;
            }
            break;
        case DefaultValue::Delegate:
            if( !Member::TypeCheck( context ) )
            {
                cppy::type_error( context, "Member" );
                return false;
            }
            break;
        case DefaultValue::CallObject:
        case DefaultValue::CallObject_Object:
        case DefaultValue::CallObject_ObjectName:
            if( !PyCallable_Check( context ) )
            {
                cppy::type_error( context, "callable" );
                return false;
            }
            break;
        case DefaultValue::ObjectMethod:
        case DefaultValue::ObjectMethod_Name:
        case DefaultValue::MemberMethod_Object:
            if( !PyUnicode_Check( context ) )
            {
                cppy::type_error( context, "str" );
                return false;
            }
            break;
        default:
            break;
    }
    return true;
}


namespace
{


PyObject*
no_op_handler( Member* member, CAtom* atom )
{
    return cppy::incref( Py_None );
}


PyObject*
static_handler( Member* member, CAtom* atom )
{
    return cppy::incref( member->default_value_context );
}


PyObject*
list_handler( Member* member, CAtom* atom )
{
    if( member->default_value_context == Py_None )
        return PyList_New( 0 );
    Py_ssize_t size = PyList_GET_SIZE( member->default_value_context );
    return PyList_GetSlice( member->default_value_context, 0, size );
}


PyObject*
set_handler( Member* member, CAtom* atom )
{
    if( member->default_value_context == Py_None )
        return PySet_New( 0 );
    return PySet_New( member->default_value_context );
}


PyObject*
dict_handler( Member* member, CAtom* atom )
{
    if( member->default_value_context == Py_None )
        return PyDict_New();
    return PyDict_Copy( member->default_value_context );
}


PyObject*
delegate_handler( Member* member, CAtom* atom )
{
    Member* delegate = member_cast( member->default_value_context );
    return delegate->default_value( atom );
}


PyObject*
non_optional_handler( Member* member, CAtom* atom )
{
    PyErr_Format(
        PyExc_ValueError,
        "The '%s' member on the '%s' object is not optional but no default value "
        "was provided and the member was not set before being accessed.",
        PyUnicode_AsUTF8( member->name ),
        Py_TYPE( pyobject_cast( atom ) )->tp_name
    );
    return 0;
}


PyObject*
call_object_handler( Member* member, CAtom* atom )
{
    cppy::ptr callable( cppy::incref( member->default_value_context ) );
    cppy::ptr args( PyTuple_New( 0 ) );
    if( !args )
        return 0;
    return callable.call( args );
}


PyObject*
call_object_object_handler( Member* member, CAtom* atom )
{
    cppy::ptr callable( cppy::incref( member->default_value_context ) );
    cppy::ptr args( PyTuple_New( 1 ) );
    if( !args )
        return 0;
    PyTuple_SET_ITEM( args.get(), 0, cppy::incref( pyobject_cast( atom ) ) );
    return callable.call( args );
}


PyObject*
call_object_object_name_handler( Member* member, CAtom* atom )
{
    cppy::ptr callable( cppy::incref( member->default_value_context ) );
    cppy::ptr args( PyTuple_New( 2 ) );
    if( !args )
        return 0;
    PyTuple_SET_ITEM( args.get(), 0, cppy::incref( pyobject_cast( atom ) ) );
    PyTuple_SET_ITEM( args.get(), 1, cppy::incref( member->name ) );
    return callable.call( args );
}


PyObject*
object_method_handler( Member* member, CAtom* atom )
{
    cppy::ptr callable( PyObject_GetAttr( pyobject_cast( atom ), member->default_value_context ) );
    if( !callable )
        return 0;
    cppy::ptr args( PyTuple_New( 0 ) );
    if( !args )
        return 0;
    return callable.call( args );
}


PyObject*
object_method_name_handler( Member* member, CAtom* atom )
{
    cppy::ptr callable( PyObject_GetAttr( pyobject_cast( atom ), member->default_value_context ) );
    if( !callable )
        return 0;
    cppy::ptr args( PyTuple_New( 1 ) );
    if( !args )
        return 0;
    PyTuple_SET_ITEM( args.get(), 0, cppy::incref( member->name ) );
    return callable.call( args );
}


PyObject*
member_method_object_handler( Member* member, CAtom* atom )
{
    cppy::ptr callable( PyObject_GetAttr( pyobject_cast( member ), member->default_value_context ) );
    if( !callable )
        return 0;
    cppy::ptr args( PyTuple_New( 1 ) );
    if( !args )
        return 0;
    PyTuple_SET_ITEM( args.get(), 0, cppy::incref( pyobject_cast( atom ) ) );
    return callable.call( args );
}


typedef PyObject*
( *handler )( Member* member, CAtom* atom );


static handler
handlers[] = {
    no_op_handler,
    static_handler,
    list_handler,
    set_handler,
    dict_handler,
    dict_handler,
    non_optional_handler,
    delegate_handler,
    call_object_handler,
    call_object_object_handler,
    call_object_object_name_handler,
    object_method_handler,
    object_method_name_handler,
    member_method_object_handler
};


}  // namespace


PyObject*
Member::default_value( CAtom* atom )
{
    if( get_default_value_mode() >= sizeof( handlers ) )
        return no_op_handler( this, atom );  // LCOV_EXCL_LINE
    return handlers[ get_default_value_mode() ]( this, atom );
}


}  // namespace atom
