/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2025, Nucleic Development Team.
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
Member::check_context( PostSetAttr::Mode mode, PyObject* context )
{
    switch( mode )
    {
        case PostSetAttr::Delegate:
            if( !Member::TypeCheck( context ) )
            {
                cppy::type_error( context, "Member" );
                return false;
            }
            break;
        case PostSetAttr::ObjectMethod_OldNew:
        case PostSetAttr::ObjectMethod_NameOldNew:
        case PostSetAttr::MemberMethod_ObjectOldNew:
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


int
no_op_handler( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    return 0;
}


int
delegate_handler( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    Member* delegate = member_cast( member->post_setattr_context );
    return delegate->post_setattr( atom, oldvalue, newvalue );
}


int
object_method_old_new_handler(
    Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    PyObject* args[] = { pyobject_cast( atom ), oldvalue, newvalue };
    cppy::ptr ok( PyObject_VectorcallMethod( member->post_setattr_context, args, 3 | PY_VECTORCALL_ARGUMENTS_OFFSET, 0 ) );
    if( !ok )
        return -1;
    return 0;
}


int
object_method_name_old_new_handler(
    Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    PyObject* args[] = { pyobject_cast( atom ), member->name, oldvalue, newvalue };
    cppy::ptr ok( PyObject_VectorcallMethod( member->post_setattr_context, args, 4 | PY_VECTORCALL_ARGUMENTS_OFFSET, 0 ) );
    if( !ok )
        return -1;
    return 0;
}


int
member_method_object_old_new_handler(
    Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    PyObject* args[] = { pyobject_cast( member ), pyobject_cast( atom ), oldvalue, newvalue };
    cppy::ptr ok( PyObject_VectorcallMethod( member->post_setattr_context, args, 4 | PY_VECTORCALL_ARGUMENTS_OFFSET, 0 ) );
    if( !ok )
        return -1;
    return 0;
}


typedef int
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


int
Member::post_setattr( CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    if( get_post_setattr_mode() >= sizeof( handlers ) )
        return no_op_handler( this, atom, oldvalue, newvalue );  // LCOV_EXCL_LINE
    return handlers[ get_post_setattr_mode() ]( this, atom, oldvalue, newvalue );
}


}  // namespace atom
