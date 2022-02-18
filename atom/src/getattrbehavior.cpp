/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2019, Atom Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file LICENSE, distributed with this software.
|----------------------------------------------------------------------------*/
#include <cppy/cppy.h>
#include "eventbinder.h"
#include "member.h"
#include "memberchange.h"
#include "signalconnector.h"

namespace atom
{


bool
Member::check_context( GetAttr::Mode mode, PyObject* context )
{
    switch( mode )
    {
        case GetAttr::Delegate:
            if( !Member::TypeCheck( context ) )
            {
                cppy::type_error( context, "Member" );
                return false;
            }
            break;
        case GetAttr::Property:
        case GetAttr::CachedProperty:
            if( context != Py_None && !PyCallable_Check( context ) )
            {
                cppy::type_error( context, "callable or None" );
                return false;
            }
            break;
        case GetAttr::CallObject_Object:
        case GetAttr::CallObject_ObjectName:
            if( !PyCallable_Check( context ) )
            {
                cppy::type_error( context, "callable" );
                return false;
            }
            break;
        case GetAttr::ObjectMethod:
        case GetAttr::ObjectMethod_Name:
        case GetAttr::MemberMethod_Object:
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
slot_handler( Member* member, CAtom* atom )
{
    if( member->index >= atom->get_slot_count() )
        return cppy::attribute_error( pyobject_cast( atom ), (char const *)PyUnicode_AsUTF8( member->name ) );
    cppy::ptr value( atom->get_slot( member->index ) );
    if( value )
    {
        if( member->get_post_getattr_mode() )
            value = member->post_getattr( atom, value.get() );
        return value.release();
    }
    value = member->default_value( atom );
    if( !value )
        return 0;
    value = member->full_validate( atom, Py_None, value.get() );
    if( !value )
        return 0;
    atom->set_slot( member->index, value.get() );
    if( atom->get_notifications_enabled() )
    {
        cppy::ptr argsptr;
        if( member->has_observers( ChangeType::Create ) )
        {
            argsptr = created_args( atom, member, value.get() );
            if( !argsptr )
                return 0;
            if( !member->notify( atom, argsptr.get(), 0, ChangeType::Create ) )
                return 0;
        }
        if( atom->has_observers( member->name ) )
        {
            if( !argsptr )
            {
                argsptr = created_args( atom, member, value.get() );
                if( !argsptr )
                    return 0;
            }
            if( !atom->notify( member->name, argsptr.get(), 0, ChangeType::Create ) )
                return 0;
        }
    }
    if( member->get_post_getattr_mode() )
        value = member->post_getattr( atom, value.get() );
    return value.release();
}


PyObject*
event_handler( Member* member, CAtom* atom )
{
    return EventBinder::New( member, atom );
}


PyObject*
signal_handler( Member* member, CAtom* atom )
{
    return SignalConnector::New( member, atom );
}


PyObject*
delegate_handler( Member* member, CAtom* atom )
{
    Member* delegate = member_cast( member->getattr_context );
    return delegate->getattr( atom );
}


PyObject*
_mangled_property_handler( Member* member, CAtom* atom )
{
    char* suffix = (char *)PyUnicode_AsUTF8( member->name );
    cppy::ptr name( PyUnicode_FromFormat( "_get_%s", suffix ) );
    if( !name )
        return 0;
    cppy::ptr callable( PyObject_GetAttr( pyobject_cast( atom ), name.get() ) );
    if( !callable )
    {
        if( PyErr_ExceptionMatches( PyExc_AttributeError ) )
            PyErr_SetString( PyExc_AttributeError, "unreadable attribute" );
        return 0;
    }
    cppy::ptr args( PyTuple_New( 0 ) );
    if( !args )
        return 0;
    return callable.call( args );
}


PyObject*
property_handler( Member* member, CAtom* atom )
{
    if( member->getattr_context != Py_None )
    {
        cppy::ptr args( PyTuple_New( 1 ) );
        if( !args )
            return 0;
        PyTuple_SET_ITEM( args.get(), 0, cppy::incref( pyobject_cast( atom ) ) );
        return PyObject_Call( member->getattr_context, args.get(), 0 );
    }
    return _mangled_property_handler( member, atom );
}


PyObject*
cached_property_handler( Member* member, CAtom* atom )
{
    cppy::ptr value( atom->get_slot( member->index ) );
    if( value )
        return value.release();
    value = property_handler( member, atom );
    atom->set_slot( member->index, value.get() );  // exception-safe
    return value.release();
}


PyObject*
call_object_object_handler( Member* member, CAtom* atom )
{
    cppy::ptr callable( cppy::incref( member->getattr_context ) );
    cppy::ptr args( PyTuple_New( 1 ) );
    if( !args )
        return 0;
    PyTuple_SET_ITEM( args.get(), 0, cppy::incref( pyobject_cast( atom ) ) );
    cppy::ptr result( callable.call( args ) );
    if( !result )
        return 0;
    return member->full_validate( atom, Py_None, result.get() );
}


PyObject*
call_object_object_name_handler( Member* member, CAtom* atom )
{
    cppy::ptr callable( cppy::incref( member->getattr_context ) );
    cppy::ptr args( PyTuple_New( 2 ) );
    if( !args )
        return 0;
    PyTuple_SET_ITEM( args.get(), 0, cppy::incref( pyobject_cast( atom ) ) );
    PyTuple_SET_ITEM( args.get(), 1, cppy::incref( member->name ) );
    cppy::ptr result( callable.call( args ) );
    if( !result )
        return 0;
    return member->full_validate( atom, Py_None, result.get() );
}


PyObject*
object_method_handler( Member* member, CAtom* atom )
{
    cppy::ptr callable( PyObject_GetAttr( pyobject_cast( atom ), member->getattr_context ) );
    if( !callable )
        return 0;
    cppy::ptr args( PyTuple_New( 0 ) );
    if( !args )
        return 0;
    cppy::ptr result( callable.call( args ) );
    if( !result )
        return 0;
    return member->full_validate( atom, Py_None, result.get() );
}


PyObject*
object_method_name_handler( Member* member, CAtom* atom )
{
    cppy::ptr callable( PyObject_GetAttr( pyobject_cast( atom ), member->getattr_context ) );
    if( !callable )
        return 0;
    cppy::ptr args( PyTuple_New( 1 ) );
    if( !args )
        return 0;
    PyTuple_SET_ITEM( args.get(), 0, cppy::incref( member->name ) );
    cppy::ptr result( callable.call( args ) );
    if( !result )
        return 0;
    return member->full_validate( atom, Py_None, result.get() );
}


PyObject*
member_method_object_handler( Member* member, CAtom* atom )
{
    cppy::ptr callable( PyObject_GetAttr( pyobject_cast( member ), member->getattr_context ) );
    if( !callable )
        return 0;
    cppy::ptr args( PyTuple_New( 1 ) );
    if( !args )
        return 0;
    PyTuple_SET_ITEM( args.get(), 0, cppy::incref( pyobject_cast( atom ) ) );
    cppy::ptr result( callable.call( args ) );
    if( !result )
        return 0;
    return member->full_validate( atom, Py_None, result.get() );
}


typedef PyObject*
( *handler )( Member* member, CAtom* atom );


static handler
handlers[] = {
    no_op_handler,
    slot_handler,
    event_handler,
    signal_handler,
    delegate_handler,
    property_handler,
    cached_property_handler,
    call_object_object_handler,
    call_object_object_name_handler,
    object_method_handler,
    object_method_name_handler,
    member_method_object_handler
};

}  // namespace


PyObject*
Member::getattr( CAtom* atom )
{
    if( get_getattr_mode() >= sizeof( handlers ) )
        return no_op_handler( this, atom );  // LCOV_EXCL_LINE
    return handlers[ get_getattr_mode() ]( this, atom );
}

}  // namespace atom
