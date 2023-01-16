/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2023, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file LICENSE, distributed with this software.
|----------------------------------------------------------------------------*/
#include <cppy/cppy.h>
#include "member.h"
#include "memberchange.h"
#include "utils.h"


namespace atom
{


bool
Member::check_context( SetAttr::Mode mode, PyObject* context )
{
    switch( mode )
    {
        case SetAttr::Delegate:
            if( !Member::TypeCheck( context ) )
            {
                cppy::type_error( context, "Member" );
                return false;
            }
            break;
        case SetAttr::Property:
            if( context != Py_None && !PyCallable_Check( context ) )
            {
                cppy::type_error( context, "callable or None" );
                return false;
            }
            break;
        case SetAttr::CallObject_ObjectValue:
        case SetAttr::CallObject_ObjectNameValue:
            if( !PyCallable_Check( context ) )
            {
                cppy::type_error( context, "callable" );
                return false;
            }
            break;
        case SetAttr::ObjectMethod_Value:
        case SetAttr::ObjectMethod_NameValue:
        case SetAttr::MemberMethod_ObjectValue:
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
no_op_handler( Member* member, CAtom* atom, PyObject* value )
{
    return 0;
}


PyObject*
created_args( CAtom* atom, Member* member, PyObject* value )
{
    cppy::ptr argsptr( PyTuple_New( 1 ) );
    if( !argsptr )
        return 0;
    cppy::ptr change( MemberChange::created( atom, member, value ) );
    if( !change )
        return 0;
    PyTuple_SET_ITEM( argsptr.get(), 0, change.release() );
    return argsptr.release();
}


PyObject*
updated_args( CAtom* atom, Member* member, PyObject* oldvalue, PyObject* newvalue )
{
    cppy::ptr argsptr( PyTuple_New( 1 ) );
    if( !argsptr )
        return 0;
    cppy::ptr change( MemberChange::updated( atom, member, oldvalue, newvalue ) );
    if( !change )
        return 0;
    PyTuple_SET_ITEM( argsptr.get(), 0, change.release() );
    return argsptr.release();
}


int
slot_handler( Member* member, CAtom* atom, PyObject* value )
{
    if( member->index >= atom->get_slot_count() )
    {
        cppy::attribute_error( pyobject_cast( atom ), (char *)PyUnicode_AsUTF8( member->name ) );
        return -1;
    }
    if( atom->is_frozen() )
    {
        PyErr_SetString( PyExc_AttributeError, "can't set attribute of frozen Atom" );
        return -1;
    }
    cppy::ptr oldptr( atom->get_slot( member->index ) );
    cppy::ptr newptr( cppy::incref( value ) );
    if( oldptr == newptr )
        return 0;
    bool valid_old = oldptr.get() != 0;
    if( !valid_old )
        oldptr.set( cppy::incref( Py_None ) );
    newptr = member->full_validate( atom, oldptr.get(), newptr.get() );
    if( !newptr )
        return -1;
    atom->set_slot( member->index, newptr.get() );
    if( member->get_post_setattr_mode() )
    {
        if( member->post_setattr( atom, oldptr.get(), newptr.get() ) < 0 )
            return -1;
    }
    if( ( !valid_old || oldptr != newptr ) && atom->get_notifications_enabled() )
    {
        cppy::ptr argsptr;
        if( member->has_observers(ChangeType::Update | ChangeType::Create) )
        {

            if( valid_old && utils::safe_richcompare( oldptr, newptr, Py_EQ ) )
                return 0;
            if( valid_old )
                argsptr = updated_args( atom, member, oldptr.get(), newptr.get() );
            else
                argsptr = created_args( atom, member, newptr.get() );
            if( !argsptr )
                return -1;
            ChangeType::Type change_type = ( valid_old ) ? ChangeType::Update: ChangeType::Create;
            if( !member->notify( atom, argsptr.get(), 0, change_type ) )
                return -1;
        }
        if( atom->has_observers( member->name ) )
        {
            ChangeType::Type change_type = ChangeType::Any;
            if( !argsptr )
            {
                if( valid_old && utils::safe_richcompare( oldptr, newptr, Py_EQ ) )
                    return 0;
                if( valid_old )
                {
                    change_type = ChangeType::Update;
                    argsptr = updated_args( atom, member, oldptr.get(), newptr.get() );
                }
                else
                {
                    change_type = ChangeType::Create;
                    argsptr = created_args( atom, member, newptr.get() );
                }
                if( !argsptr )
                    return -1;
            }
            if( !atom->notify( member->name, argsptr.get(), 0, change_type ) )
                return -1;
        }
    }
    return 0;
}


int
constant_handler( Member* member, CAtom* atom, PyObject* value )
{
    cppy::type_error( "cannot set the value of a constant member" );
    return -1;
}


int
read_only_handler( Member* member, CAtom* atom, PyObject* value )
{
    if( member->index >= atom->get_slot_count() )
    {
        cppy::attribute_error( pyobject_cast( atom ), (char *)PyUnicode_AsUTF8( member->name ) );
        return -1;
    }
    cppy::ptr slot( atom->get_slot( member->index ) );
    if( slot )
    {
        cppy::type_error( "cannot change the value of a read only member" );
        return -1;
    }
    return slot_handler( member, atom, value );
}


PyObject*
event_args( CAtom* atom, Member* member, PyObject* value )
{
    cppy::ptr argsptr( PyTuple_New( 1 ) );
    if( !argsptr )
        return 0;
    cppy::ptr change( MemberChange::event( atom, member, value ) );
    if( !change )
        return 0;
    PyTuple_SET_ITEM( argsptr.get(), 0, change.release() );
    return argsptr.release();
}


int
event_handler( Member* member, CAtom* atom, PyObject* value )
{
    cppy::ptr valueptr( member->full_validate( atom, Py_None, value ) );
    if( !valueptr )
        return -1;
    if( atom->get_notifications_enabled() )
    {
        cppy::ptr argsptr;
        if( member->has_observers( ChangeType::Event ) )
        {
            argsptr = event_args( atom, member, valueptr.get() );
            if( !argsptr )
                return -1;
            if( !member->notify( atom, argsptr.get(), 0, ChangeType::Event ) )
                return -1;
        }
        if( atom->has_observers( member->name ) )
        {
            if( !argsptr )
            {
                argsptr = event_args( atom, member, valueptr.get() );
                if( !argsptr )
                    return -1;
            }
            if( !atom->notify( member->name, argsptr.get(), 0, ChangeType::Event ) )
                return -1;
        }
    }
    return 0;
}


int
signal_handler( Member* member, CAtom* atom, PyObject* value )
{
    cppy::type_error( "cannot set the value of a signal" );
    return -1;
}


int
delegate_handler( Member* member, CAtom* atom, PyObject* value )
{
    Member* delegate = member_cast( member->setattr_context );
    return delegate->setattr( atom, value );
}


int
_mangled_property_handler( Member* member, CAtom* atom, PyObject* value )
{
    char* suffix = (char *)PyUnicode_AsUTF8( member->name );
    cppy::ptr name( PyUnicode_FromFormat( "_set_%s", suffix ) );
    if( !name )
        return -1;
    cppy::ptr callable( PyObject_GetAttr( pyobject_cast( atom ), name.get() ) );
    if( !callable )
    {
        if( PyErr_ExceptionMatches( PyExc_AttributeError ) )
            PyErr_SetString( PyExc_AttributeError, "can't set attribute" );
        return -1;
    }
    cppy::ptr argsptr( PyTuple_New( 1 ) );
    if( !argsptr )
        return -1;
    PyTuple_SET_ITEM( argsptr.get(), 0, cppy::incref( value ) );
    cppy::ptr ok( PyObject_Call( callable.get(), argsptr.get(), 0 ) );
    if( !ok )
        return -1;
    return 0;
}


int
property_handler( Member* member, CAtom* atom, PyObject* value )
{
    if( member->setattr_context != Py_None )
    {
        cppy::ptr argsptr( PyTuple_New( 2 ) );
        if( !argsptr )
            return -1;
        PyTuple_SET_ITEM( argsptr.get(), 0, cppy::incref( pyobject_cast( atom ) ) );
        PyTuple_SET_ITEM( argsptr.get(), 1, cppy::incref( pyobject_cast( value ) ) );
        cppy::ptr ok( PyObject_Call( member->setattr_context, argsptr.get(), 0 ) );
        if( !ok )
            return -1;
        return 0;
    }
    return _mangled_property_handler( member, atom, value );
}


int
call_object_object_value_handler( Member* member, CAtom* atom, PyObject* value )
{
    cppy::ptr valueptr( cppy::incref( value ) );
    valueptr = member->full_validate( atom, Py_None, valueptr.get() );
    if( !valueptr )
        return -1;
    cppy::ptr callable( cppy::incref( member->setattr_context ) );
    cppy::ptr argsptr( PyTuple_New( 2 ) );
    if( !argsptr )
        return -1;
    PyTuple_SET_ITEM( argsptr.get(), 0, cppy::incref( pyobject_cast( atom ) ) );
    PyTuple_SET_ITEM( argsptr.get(), 1, valueptr.release() );
    if( !callable.call( argsptr ) )
        return -1;
    return 0;
}


int
call_object_object_name_value_handler( Member* member, CAtom* atom, PyObject* value )
{
    cppy::ptr valueptr( cppy::incref( value ) );
    valueptr = member->full_validate( atom, Py_None, valueptr.get() );
    if( !valueptr )
        return -1;
    cppy::ptr callable( cppy::incref( member->setattr_context ) );
    cppy::ptr argsptr( PyTuple_New( 3 ) );
    if( !argsptr )
        return -1;
    PyTuple_SET_ITEM( argsptr.get(), 0, cppy::incref( pyobject_cast( atom ) ) );
    PyTuple_SET_ITEM( argsptr.get(), 1, cppy::incref( member->name ) );
    PyTuple_SET_ITEM( argsptr.get(), 2, valueptr.release() );
    if( !callable.call( argsptr ) )
        return -1;
    return 0;
}


int
object_method_value_handler( Member* member, CAtom* atom, PyObject* value )
{
    cppy::ptr valueptr( cppy::incref( value ) );
    valueptr = member->full_validate( atom, Py_None, valueptr.get() );
    if( !valueptr )
        return -1;
    cppy::ptr callable( PyObject_GetAttr( pyobject_cast( atom ), member->setattr_context ) );
    if( !callable )
        return -1;
    cppy::ptr argsptr( PyTuple_New( 1 ) );
    if( !argsptr )
        return -1;
    PyTuple_SET_ITEM( argsptr.get(), 0, valueptr.release() );
    if( !callable.call( argsptr ) )
        return -1;
    return 0;
}


int
object_method_name_value_handler( Member* member, CAtom* atom, PyObject* value )
{
    cppy::ptr valueptr( cppy::incref( value ) );
    valueptr = member->full_validate( atom, Py_None, valueptr.get() );
    if( !valueptr )
        return -1;
    cppy::ptr callable( PyObject_GetAttr( pyobject_cast( atom ), member->setattr_context ) );
    if( !callable )
        return -1;
    cppy::ptr argsptr( PyTuple_New( 2 ) );
    if( !argsptr )
        return -1;
    PyTuple_SET_ITEM( argsptr.get(), 0, cppy::incref( member->name ) );
    PyTuple_SET_ITEM( argsptr.get(), 1, valueptr.release() );
    if( !callable.call( argsptr ) )
        return -1;
    return 0;
}


int
member_method_object_value_handler( Member* member, CAtom* atom, PyObject* value )
{
    cppy::ptr valueptr( cppy::incref( value ) );
    valueptr = member->full_validate( atom, Py_None, valueptr.get() );
    if( !valueptr )
        return -1;
    cppy::ptr callable( PyObject_GetAttr( pyobject_cast( member ), member->setattr_context ) );
    if( !callable )
        return -1;
    cppy::ptr argsptr( PyTuple_New( 2 ) );
    if( !argsptr )
        return -1;
    PyTuple_SET_ITEM( argsptr.get(), 0, cppy::incref( pyobject_cast( atom ) ) );
    PyTuple_SET_ITEM( argsptr.get(), 1, valueptr.release() );
    if( !callable.call( argsptr ) )
        return -1;
    return 0;
}


typedef int
( *handler )( Member* member, CAtom* atom, PyObject* value );


static handler
handlers[] = {
    no_op_handler,
    slot_handler,
    constant_handler,
    read_only_handler,
    event_handler,
    signal_handler,
    delegate_handler,
    property_handler,
    call_object_object_value_handler,
    call_object_object_name_value_handler,
    object_method_value_handler,
    object_method_name_value_handler,
    member_method_object_value_handler
};


} // namespace


int
Member::setattr( CAtom* atom, PyObject* value )
{
    if( get_setattr_mode() >= sizeof( handlers ) )
        return no_op_handler( this, atom, value );  // LCOV_EXCL_LINE
    return handlers[ get_setattr_mode() ]( this, atom, value );
}


}  // namespace atom
