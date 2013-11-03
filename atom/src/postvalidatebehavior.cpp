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
Member::check_context( PostValidate::Mode mode, PyObject* context )
{
    switch( mode )
    {
        case PostValidate::Delegate:
            if( !Member::TypeCheck( context ) )
            {
                py_expected_type_fail( context, "Member" );
                return false;
            }
            break;
        case PostValidate::ObjectMethod_OldNew:
        case PostValidate::ObjectMethod_NameOldNew:
        case PostValidate::MemberMethod_ObjectOldNew:
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
no_op_handler( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    return newref( newvalue );
}


static PyObject*
delegate_handler( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    Member* delegate = member_cast( member->post_validate_context );
    return delegate->post_validate( atom, oldvalue, newvalue );
}


static PyObject*
object_method_old_new_handler(
    Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    PyObjectPtr callable( PyObject_GetAttr( pyobject_cast( atom ), member->post_validate_context ) );
    if( !callable )
        return 0;
    PyTuplePtr args( PyTuple_New( 2 ) );
    if( !args )
        return 0;
    args.initialize( 0, newref( oldvalue ) );
    args.initialize( 1, newref( newvalue ) );
    return callable( args ).release();
}


static PyObject*
object_method_name_old_new_handler(
    Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    PyObjectPtr callable( PyObject_GetAttr( pyobject_cast( atom ), member->post_validate_context ) );
    if( !callable )
        return 0;
    PyTuplePtr args( PyTuple_New( 3 ) );
    if( !args )
        return 0;
    args.initialize( 0, newref( member->name ) );
    args.initialize( 1, newref( oldvalue ) );
    args.initialize( 2, newref( newvalue ) );
    return callable( args ).release();
}


static PyObject*
member_method_object_old_new_handler(
    Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    PyObjectPtr callable( PyObject_GetAttr( pyobject_cast( member ), member->post_validate_context ) );
    if( !callable )
        return 0;
    PyTuplePtr args( PyTuple_New( 3 ) );
    if( !args )
        return 0;
    args.initialize( 0, newref( pyobject_cast( atom ) ) );
    args.initialize( 1, newref( oldvalue ) );
    args.initialize( 2, newref( newvalue ) );
    return callable( args ).release();
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


PyObject*
Member::post_validate( CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    if( get_post_validate_mode() >= sizeof( handlers ) )
        return no_op_handler( this, atom, oldvalue, newvalue );
    return handlers[ get_post_validate_mode() ]( this, atom, oldvalue, newvalue );
}
