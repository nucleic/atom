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
Member::check_context( PostValidate::Mode mode, PyObject* context )
{
    switch( mode )
    {
        case PostValidate::Delegate:
            if( !Member::TypeCheck( context ) )
            {
                cppy::type_error( context, "Member" );
                return false;
            }
            break;
        case PostValidate::ObjectMethod_OldNew:
        case PostValidate::ObjectMethod_NameOldNew:
        case PostValidate::MemberMethod_ObjectOldNew:
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
no_op_handler( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    return cppy::incref( newvalue );
}


PyObject*
delegate_handler( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    Member* delegate = member_cast( member->post_validate_context );
    return delegate->post_validate( atom, oldvalue, newvalue );
}


PyObject*
object_method_old_new_handler(
    Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    cppy::ptr callable( PyObject_GetAttr( pyobject_cast( atom ), member->post_validate_context ) );
    if( !callable )
        return 0;
    cppy::ptr args( PyTuple_New( 2 ) );
    if( !args )
        return 0;
    PyTuple_SET_ITEM( args.get(), 0, cppy::incref( oldvalue ) );
    PyTuple_SET_ITEM( args.get(), 1, cppy::incref( newvalue ) );
    return callable.call( args );
}


PyObject*
object_method_name_old_new_handler(
    Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    cppy::ptr callable( PyObject_GetAttr( pyobject_cast( atom ), member->post_validate_context ) );
    if( !callable )
        return 0;
    cppy::ptr args( PyTuple_New( 3 ) );
    if( !args )
        return 0;
    PyTuple_SET_ITEM( args.get(), 0, cppy::incref( member->name ) );
    PyTuple_SET_ITEM( args.get(), 1, cppy::incref( oldvalue ) );
    PyTuple_SET_ITEM( args.get(), 2, cppy::incref( newvalue ) );
    return callable.call( args );
}


PyObject*
member_method_object_old_new_handler(
    Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    cppy::ptr callable( PyObject_GetAttr( pyobject_cast( member ), member->post_validate_context ) );
    if( !callable )
        return 0;
    cppy::ptr args( PyTuple_New( 3 ) );
    if( !args )
        return 0;
    PyTuple_SET_ITEM( args.get(), 0, cppy::incref( pyobject_cast( atom ) ) );
    PyTuple_SET_ITEM( args.get(), 1, cppy::incref( oldvalue ) );
    PyTuple_SET_ITEM( args.get(), 2, cppy::incref( newvalue ) );
    return callable.call( args );
}


typedef PyObject*
( *handler )( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue );


static handler
handlers[] = {
    no_op_handler,
    delegate_handler,
    object_method_old_new_handler,
    object_method_name_old_new_handler,
    member_method_object_old_new_handler
};


}  // namespace


PyObject*
Member::post_validate( CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    if( get_post_validate_mode() >= sizeof( handlers ) )
        return no_op_handler( this, atom, oldvalue, newvalue );  // LCOV_EXCL_LINE
    return handlers[ get_post_validate_mode() ]( this, atom, oldvalue, newvalue );
}

}  // namespace atom
