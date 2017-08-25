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
Member::check_context( PostGetAttr::Mode mode, PyObject* context )
{
    switch( mode )
    {
        case PostGetAttr::Delegate:
            if( !Member::TypeCheck( context ) )
            {
                py_expected_type_fail( context, "Member" );
                return false;
            }
            break;
        case PostGetAttr::ObjectMethod_Value:
        case PostGetAttr::ObjectMethod_NameValue:
        case PostGetAttr::MemberMethod_ObjectValue:
            if( !PyBytes_Check( context ) )
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
no_op_handler( Member* member, CAtom* atom, PyObject* value )
{
    return newref( value );
}


static PyObject*
delegate_handler( Member* member, CAtom* atom, PyObject* value )
{
    Member* delegate = member_cast( member->post_getattr_context );
    return delegate->post_getattr( atom, value );
}


static PyObject*
object_method_value_handler( Member* member, CAtom* atom, PyObject* value )
{
    PyObjectPtr callable( PyObject_GetAttr( pyobject_cast( atom ), member->post_getattr_context ) );
    if( !callable )
        return 0;
    PyTuplePtr args( PyTuple_New( 1 ) );
    if( !args )
        return 0;
    args.initialize( 0, newref( value ) );
    return callable( args ).release();
}


static PyObject*
object_method_name_value_handler( Member* member, CAtom* atom, PyObject* value )
{
    PyObjectPtr callable( PyObject_GetAttr( pyobject_cast( atom ), member->post_getattr_context ) );
    if( !callable )
        return 0;
    PyTuplePtr args( PyTuple_New( 2 ) );
    if( !args )
        return 0;
    args.initialize( 0, newref( member->name ) );
    args.initialize( 1, newref( value ) );
    return callable( args ).release();
}


static PyObject*
member_method_object_value_handler( Member* member, CAtom* atom, PyObject* value )
{
    PyObjectPtr callable( PyObject_GetAttr( pyobject_cast( member ), member->post_getattr_context ) );
    if( !callable )
        return 0;
    PyTuplePtr args( PyTuple_New( 2 ) );
    if( !args )
        return 0;
    args.initialize( 0, newref( pyobject_cast( atom ) ) );
    args.initialize( 1, newref( value ) );
    return callable( args ).release();
}


typedef PyObject*
( *handler )( Member* member, CAtom* atom, PyObject* value );


static handler
handlers[] = {
    no_op_handler,
    delegate_handler,
    object_method_value_handler,
    object_method_name_value_handler,
    member_method_object_value_handler
};


PyObject*
Member::post_getattr( CAtom* atom, PyObject* value )
{
    if( get_post_getattr_mode() >= sizeof( handlers ) )
        return no_op_handler( this, atom, value );
    return handlers[ get_post_getattr_mode() ]( this, atom, value );
}
