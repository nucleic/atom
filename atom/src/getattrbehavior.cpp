/*-----------------------------------------------------------------------------
| Copyright (c) 2013, Atom Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include "eventbinder.h"
#include "member.h"
#include "memberchange.h"
#include "signalconnector.h"


using namespace PythonHelpers;


bool
Member::check_context( GetAttr::Mode mode, PyObject* context )
{
    switch( mode )
    {
        case GetAttr::Delegate:
            if( !Member::TypeCheck( context ) )
            {
                py_expected_type_fail( context, "Member" );
                return false;
            }
            break;
        case GetAttr::Property:
        case GetAttr::CachedProperty:
            if( context != Py_None && !PyCallable_Check( context ) )
            {
                py_expected_type_fail( context, "callable or None" );
                return false;
            }
            break;
        case GetAttr::CallObject_Object:
        case GetAttr::CallObject_ObjectName:
            if( !PyCallable_Check( context ) )
            {
                py_expected_type_fail( context, "callable" );
                return false;
            }
            break;
        case GetAttr::ObjectMethod:
        case GetAttr::ObjectMethod_Name:
        case GetAttr::MemberMethod_Object:
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
no_op_handler( Member* member, CAtom* atom )
{
    return newref( Py_None );
}


static PyObject*
created_args( CAtom* atom, Member* member, PyObject* value )
{
    PyTuplePtr argsptr( PyTuple_New( 1 ) );
    if( !argsptr )
        return 0;
    PyObjectPtr changeptr( MemberChange::created( atom, member, value ) );
    if( !changeptr )
        return 0;
    argsptr.initialize( 0, changeptr );
    return argsptr.release();
}


static PyObject*
slot_handler( Member* member, CAtom* atom )
{
    if( member->index >= atom->get_slot_count() )
        return py_no_attr_fail( pyobject_cast( atom ), PyBytes_AsString( member->name ) );
    PyObjectPtr value( atom->get_slot( member->index ) );
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
        PyObjectPtr argsptr;
        if( member->has_observers() )
        {
            argsptr = created_args( atom, member, value.get() );
            if( !argsptr )
                return 0;
            if( !member->notify( atom, argsptr.get(), 0 ) )
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
            if( !atom->notify( member->name, argsptr.get(), 0 ) )
                return 0;
        }
    }
    if( member->get_post_getattr_mode() )
        value = member->post_getattr( atom, value.get() );
    return value.release();
}


static PyObject*
event_handler( Member* member, CAtom* atom )
{
    return EventBinder_New( member, atom );
}


static PyObject*
signal_handler( Member* member, CAtom* atom )
{
    return SignalConnector_New( member, atom );
}


static PyObject*
delegate_handler( Member* member, CAtom* atom )
{
    Member* delegate = member_cast( member->getattr_context );
    return delegate->getattr( atom );
}


static PyObject*
_mangled_property_handler( Member* member, CAtom* atom )
{
    char* suffix = PyString_AS_STRING( member->name );
    PyObjectPtr name( PyString_FromFormat( "_get_%s", suffix ) );
    if( !name )
        return 0;
    PyObjectPtr callable( PyObject_GetAttr( pyobject_cast( atom ), name.get() ) );
    if( !callable )
    {
        if( PyErr_ExceptionMatches( PyExc_AttributeError ) )
            PyErr_SetString( PyExc_AttributeError, "unreadable attribute" );
        return 0;
    }
    PyObjectPtr args( PyTuple_New( 0 ) );
    if( !args )
        return 0;
    return PyObject_Call( callable.get(), args.get(), 0 );
}


static PyObject*
property_handler( Member* member, CAtom* atom )
{
    if( member->getattr_context != Py_None )
    {
        PyObjectPtr args( PyTuple_New( 1 ) );
        if( !args )
            return 0;
        PyTuple_SET_ITEM( args.get(), 0, newref( pyobject_cast( atom ) ) );
        return PyObject_Call( member->getattr_context, args.get(), 0 );
    }
    return _mangled_property_handler( member, atom );
}


static PyObject*
cached_property_handler( Member* member, CAtom* atom )
{
    PyObjectPtr value( atom->get_slot( member->index ) );
    if( value )
        return value.release();
    value = property_handler( member, atom );
    atom->set_slot( member->index, value.get() );  // exception-safe
    return value.release();
}


static PyObject*
call_object_object_handler( Member* member, CAtom* atom )
{
    PyObjectPtr callable( newref( member->getattr_context ) );
    PyTuplePtr args( PyTuple_New( 1 ) );
    if( !args )
        return 0;
    args.initialize( 0, newref( pyobject_cast( atom ) ) );
    PyObjectPtr result( callable( args ) );
    if( !result )
        return 0;
    return member->full_validate( atom, Py_None, result.get() );
}


static PyObject*
call_object_object_name_handler( Member* member, CAtom* atom )
{
    PyObjectPtr callable( newref( member->getattr_context ) );
    PyTuplePtr args( PyTuple_New( 2 ) );
    if( !args )
        return 0;
    args.initialize( 0, newref( pyobject_cast( atom ) ) );
    args.initialize( 1, newref( member->name ) );
    PyObjectPtr result( callable( args ) );
    if( !result )
        return 0;
    return member->full_validate( atom, Py_None, result.get() );
}


static PyObject*
object_method_handler( Member* member, CAtom* atom )
{
    PyObjectPtr callable( PyObject_GetAttr( pyobject_cast( atom ), member->getattr_context ) );
    if( !callable )
        return 0;
    PyTuplePtr args( PyTuple_New( 0 ) );
    if( !args )
        return 0;
    PyObjectPtr result( callable( args ) );
    if( !result )
        return 0;
    return member->full_validate( atom, Py_None, result.get() );
}


static PyObject*
object_method_name_handler( Member* member, CAtom* atom )
{
    PyObjectPtr callable( PyObject_GetAttr( pyobject_cast( atom ), member->getattr_context ) );
    if( !callable )
        return 0;
    PyTuplePtr args( PyTuple_New( 1 ) );
    if( !args )
        return 0;
    args.initialize( 0, newref( member->name ) );
    PyObjectPtr result( callable( args ) );
    if( !result )
        return 0;
    return member->full_validate( atom, Py_None, result.get() );
}


static PyObject*
member_method_object_handler( Member* member, CAtom* atom )
{
    PyObjectPtr callable( PyObject_GetAttr( pyobject_cast( member ), member->getattr_context ) );
    if( !callable )
        return 0;
    PyTuplePtr args( PyTuple_New( 1 ) );
    if( !args )
        return 0;
    args.initialize( 0, newref( pyobject_cast( atom ) ) );
    PyObjectPtr result( callable( args ) );
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


PyObject*
Member::getattr( CAtom* atom )
{
    if( get_getattr_mode() >= sizeof( handlers ) )
        return no_op_handler( this, atom );
    return handlers[ get_getattr_mode() ]( this, atom );
}
