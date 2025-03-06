/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2024, Nucleic Development Team.
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
    PyObject* args[] = { pyobject_cast( atom ), oldvalue, newvalue };
    return PyObject_VectorcallMethod( member->post_validate_context, args, 3 | PY_VECTORCALL_ARGUMENTS_OFFSET, 0 );
}


PyObject*
object_method_name_old_new_handler(
    Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    PyObject* args[] = { pyobject_cast( atom ), member->name, oldvalue, newvalue };
    return PyObject_VectorcallMethod( member->post_validate_context, args, 4 | PY_VECTORCALL_ARGUMENTS_OFFSET, 0 );
}


PyObject*
member_method_object_old_new_handler(
    Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    PyObject* args[] = { pyobject_cast( member ), pyobject_cast( atom ), oldvalue, newvalue };
    return PyObject_VectorcallMethod( member->post_validate_context, args, 4 | PY_VECTORCALL_ARGUMENTS_OFFSET, 0 );
}


typedef PyObject*
( *handler )( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue );


static handler
handlers[] = {
    no_op_handler,
    delegate_handler,
    object_method_old_new_handler,
    object_method_name_old_new_handler,
    member_method_object_old_new_handler,
    no_op_handler,
    no_op_handler,
    no_op_handler
};

const auto mask = validate_handlers(handlers, PostValidate::Mode::Last);

}  // namespace


PyObject*
Member::post_validate( CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    return handlers[ get_post_validate_mode() & mask ]( this, atom, oldvalue, newvalue );
}

}  // namespace atom
