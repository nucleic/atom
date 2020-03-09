/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2019, Nucleic Development Team.
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
Member::check_context( PostGetAttr::Mode mode, PyObject* context )
{
    switch( mode )
    {
        case PostGetAttr::Delegate:
            if( !Member::TypeCheck( context ) )
            {
                cppy::type_error( context, "Member" );
                return false;
            }
            break;
        case PostGetAttr::ObjectMethod_Value:
        case PostGetAttr::ObjectMethod_NameValue:
        case PostGetAttr::MemberMethod_ObjectValue:
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
no_op_handler( Member* member, CAtom* atom, PyObject* value )
{
    return cppy::incref( value );
}


PyObject*
delegate_handler( Member* member, CAtom* atom, PyObject* value )
{
    Member* delegate = member_cast( member->post_getattr_context );
    return delegate->post_getattr( atom, value );
}


PyObject*
object_method_value_handler( Member* member, CAtom* atom, PyObject* value )
{
    cppy::ptr callable( PyObject_GetAttr( pyobject_cast( atom ), member->post_getattr_context ) );
    if( !callable )
        return 0;
    cppy::ptr args( PyTuple_New( 1 ) );
    if( !args )
        return 0;
    PyTuple_SET_ITEM( args.get(), 0, cppy::incref( value ) );
    return callable.call( args );
}


PyObject*
object_method_name_value_handler( Member* member, CAtom* atom, PyObject* value )
{
    cppy::ptr callable( PyObject_GetAttr( pyobject_cast( atom ), member->post_getattr_context ) );
    if( !callable )
        return 0;
    cppy::ptr args( PyTuple_New( 2 ) );
    if( !args )
        return 0;
    PyTuple_SET_ITEM( args.get(), 0, cppy::incref( member->name ) );
    PyTuple_SET_ITEM( args.get(), 1, cppy::incref( value ) );
    return callable.call( args );
}


PyObject*
member_method_object_value_handler( Member* member, CAtom* atom, PyObject* value )
{
    cppy::ptr callable( PyObject_GetAttr( pyobject_cast( member ), member->post_getattr_context ) );
    if( !callable )
        return 0;
    cppy::ptr args( PyTuple_New( 2 ) );
    if( !args )
        return 0;
    PyTuple_SET_ITEM( args.get(), 0, cppy::incref( pyobject_cast( atom ) ) );
    PyTuple_SET_ITEM( args.get(), 1, cppy::incref( value ) );
    return callable.call( args );
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


}  // namespace


PyObject*
Member::post_getattr( CAtom* atom, PyObject* value )
{
    if( get_post_getattr_mode() >= sizeof( handlers ) )
        return no_op_handler( this, atom, value );  // LCOV_EXCL_LINE
    return handlers[ get_post_getattr_mode() ]( this, atom, value );
}


}  // namespace atom
