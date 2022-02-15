/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2019, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file LICENSE, distributed with this software.
|----------------------------------------------------------------------------*/
#include <cppy/cppy.h>
#include "member.h"
#include "memberchange.h"


namespace atom
{


bool
Member::check_context( DelAttr::Mode mode, PyObject* context )
{
    switch( mode )
    {
        case DelAttr::Delegate:
            if( !Member::TypeCheck( context ) )
            {
                cppy::type_error( context, "Member" );
                return false;
            }
            break;
        case DelAttr::Property:
            if( context != Py_None && !PyCallable_Check( context ) )
            {
                cppy::type_error( context, "callable or None" );
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
no_op_handler( Member* member, CAtom* atom )
{
    return 0;
}


PyObject*
deleted_args( CAtom* atom, Member* member, PyObject* value )
{
    cppy::ptr argsptr( PyTuple_New( 1 ) );
    if( !argsptr )
        return 0;
    cppy::ptr change( MemberChange::deleted( atom, member, value ) );
    if( !change )
        return 0;
    PyTuple_SET_ITEM( argsptr.get(), 0, change.release() );
    return argsptr.release();
}


int
slot_handler( Member* member, CAtom* atom )
{
    if( member->index >= atom->get_slot_count() )
    {
        cppy::attribute_error( pyobject_cast( atom ), (char const *)PyUnicode_AsUTF8( member->name ) );
        return -1;
    }
    if( atom->is_frozen() )
    {
        PyErr_SetString( PyExc_AttributeError, "can't delete attribute of frozen Atom" );
        return -1;
    }
    cppy::ptr valueptr( atom->get_slot( member->index ) );
    if( !valueptr )
        return 0;
    atom->set_slot( member->index, 0 );
    if( atom->get_notifications_enabled() )
    {
        cppy::ptr argsptr;
        if( member->has_observers( ChangeType::Delete ) )
        {
            argsptr = deleted_args( atom, member, valueptr.get() );
            if( !argsptr )
                return -1;
            if( !member->notify( atom, argsptr.get(), 0, ChangeType::Delete ) )
                return -1;
        }
        if( atom->has_observers( member->name ) )
        {
            if( !argsptr )
            {
                argsptr = deleted_args( atom, member, valueptr.get() );
                if( !argsptr )
                    return -1;
            }
            if( !atom->notify( member->name, argsptr.get(), 0, ChangeType::Delete ) )
                return -1;
        }
    }
    return 0;
}


int
constant_handler( Member* member, CAtom* atom )
{
    cppy::type_error( "cannot delete the value of a constant member" );
    return -1;
}


int
read_only_handler( Member* member, CAtom* atom )
{
    cppy::type_error( "cannot delete the value of a read only member" );
    return -1;
}


int
event_handler( Member* member, CAtom* atom )
{
    cppy::type_error( "cannot delete the value of an event" );
    return -1;
}


int
signal_handler( Member* member, CAtom* atom )
{
    cppy::type_error( "cannot delete the value of a signal" );
    return -1;
}


int
delegate_handler( Member* member, CAtom* atom )
{
    Member* delegate = member_cast( member->delattr_context );
    return delegate->delattr( atom );
}


int
_mangled_property_handler( Member* member, CAtom* atom )
{
    char* suffix = (char *)PyUnicode_AsUTF8( member->name );
    cppy::ptr name( PyUnicode_FromFormat( "_del_%s", suffix ) );
    if( !name )
        return -1;
    cppy::ptr callable( PyObject_GetAttr( pyobject_cast( atom ), name.get() ) );
    if( !callable )
    {
        if( PyErr_ExceptionMatches( PyExc_AttributeError ) )
            PyErr_SetString( PyExc_AttributeError, "can't delete attribute" );
        return -1;
    }
    cppy::ptr args( PyTuple_New( 0 ) );
    if( !args )
        return -1;
    cppy::ptr ok( callable.call( args ) );
    if( !ok )
        return -1;
    return 0;
}


int
property_handler( Member* member, CAtom* atom )
{
    if( member->delattr_context != Py_None )
    {
        cppy::ptr args( PyTuple_New( 1 ) );
        if( !args )
            return -1;
        PyTuple_SET_ITEM( args.get(), 0, cppy::incref( pyobject_cast( atom ) ) );
        cppy::ptr ok( PyObject_Call( member->delattr_context, args.get(), 0 ) );
        if( !ok )
            return -1;
        return 0;
    }
    return _mangled_property_handler( member, atom );
}


typedef int
( *handler )( Member* member, CAtom* atom );


static handler
handlers[] = {
    no_op_handler,
    slot_handler,
    constant_handler,
    read_only_handler,
    event_handler,
    signal_handler,
    delegate_handler,
    property_handler
};


}  // namespace


int
Member::delattr( CAtom* atom )
{
    if( get_delattr_mode() >= sizeof( handlers ) )
        return no_op_handler( this, atom );  // LCOV_EXCL_LINE
    return handlers[ get_delattr_mode() ]( this, atom );
}


}  // namespace atom
