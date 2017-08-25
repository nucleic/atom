/*-----------------------------------------------------------------------------
| Copyright (c) 2013, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include "member.h"
#include "memberchange.h"
#include "py23compat.h"


using namespace PythonHelpers;


bool
Member::check_context( SetAttr::Mode mode, PyObject* context )
{
    switch( mode )
    {
        case SetAttr::Delegate:
            if( !Member::TypeCheck( context ) )
            {
                py_expected_type_fail( context, "Member" );
                return false;
            }
            break;
        case SetAttr::Property:
            if( context != Py_None && !PyCallable_Check( context ) )
            {
                py_expected_type_fail( context, "callable or None" );
                return false;
            }
            break;
        case SetAttr::CallObject_ObjectValue:
        case SetAttr::CallObject_ObjectNameValue:
            if( !PyCallable_Check( context ) )
            {
                py_expected_type_fail( context, "callable" );
                return false;
            }
            break;
        case SetAttr::ObjectMethod_Value:
        case SetAttr::ObjectMethod_NameValue:
        case SetAttr::MemberMethod_ObjectValue:
            if( !Py23Str_Check( context ) )
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


static int
no_op_handler( Member* member, CAtom* atom, PyObject* value )
{
    return 0;
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
updated_args( CAtom* atom, Member* member, PyObject* oldvalue, PyObject* newvalue )
{
    PyTuplePtr argsptr( PyTuple_New( 1 ) );
    if( !argsptr )
        return 0;
    PyObjectPtr changeptr( MemberChange::updated( atom, member, oldvalue, newvalue ) );
    if( !changeptr )
        return 0;
    argsptr.initialize( 0, changeptr );
    return argsptr.release();
}


static int
slot_handler( Member* member, CAtom* atom, PyObject* value )
{
    if( member->index >= atom->get_slot_count() )
    {
        py_no_attr_fail( pyobject_cast( atom ), (char *)Py23Str_AS_STRING( member->name ) );
        return -1;
    }
    if( atom->is_frozen() )
    {
        PyErr_SetString( PyExc_AttributeError, "can't set attribute of frozen Atom" );
        return -1;
    }
    PyObjectPtr oldptr( atom->get_slot( member->index ) );
    PyObjectPtr newptr( newref( value ) );
    if( oldptr == newptr )
        return 0;
    bool valid_old = oldptr.get() != 0;
    if( !valid_old )
        oldptr.set( newref( Py_None ) );
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
        PyObjectPtr argsptr;
        if( member->has_observers() )
        {
            if( valid_old && oldptr.richcompare( newptr, Py_EQ ) )
                return 0;
            if( valid_old )
                argsptr = updated_args( atom, member, oldptr.get(), newptr.get() );
            else
                argsptr = created_args( atom, member, newptr.get() );
            if( !argsptr )
                return -1;
            if( !member->notify( atom, argsptr.get(), 0 ) )
                return -1;
        }
        if( atom->has_observers( member->name ) )
        {
            if( !argsptr )
            {
                if( valid_old && oldptr.richcompare( newptr, Py_EQ ) )
                    return 0;
                if( valid_old )
                    argsptr = updated_args( atom, member, oldptr.get(), newptr.get() );
                else
                    argsptr = created_args( atom, member, newptr.get() );
                if( !argsptr )
                    return -1;
            }
            if( !atom->notify( member->name, argsptr.get(), 0 ) )
                return -1;
        }
    }
    return 0;
}


static int
constant_handler( Member* member, CAtom* atom, PyObject* value )
{
    py_type_fail( "cannot set the value of a constant member" );
    return -1;
}


static int
read_only_handler( Member* member, CAtom* atom, PyObject* value )
{
    if( member->index >= atom->get_slot_count() )
    {
        py_no_attr_fail( pyobject_cast( atom ), (char *)Py23Str_AS_STRING( member->name ) );
        return -1;
    }
    PyObjectPtr slot( atom->get_slot( member->index ) );
    if( slot )
    {
        py_type_fail( "cannot change the value of a read only member" );
        return -1;
    }
    return slot_handler( member, atom, value );
}


static PyObject*
event_args( CAtom* atom, Member* member, PyObject* value )
{
    PyTuplePtr argsptr( PyTuple_New( 1 ) );
    if( !argsptr )
        return 0;
    PyObjectPtr changeptr( MemberChange::event( atom, member, value ) );
    if( !changeptr )
        return 0;
    argsptr.initialize( 0, changeptr );
    return argsptr.release();
}


static int
event_handler( Member* member, CAtom* atom, PyObject* value )
{
    PyObjectPtr valueptr( member->full_validate( atom, Py_None, value ) );
    if( !valueptr )
        return -1;
    if( atom->get_notifications_enabled() )
    {
        PyObjectPtr argsptr;
        if( member->has_observers() )
        {
            argsptr = event_args( atom, member, valueptr.get() );
            if( !argsptr )
                return -1;
            if( !member->notify( atom, argsptr.get(), 0 ) )
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
            if( !atom->notify( member->name, argsptr.get(), 0 ) )
                return -1;
        }
    }
    return 0;
}


static int
signal_handler( Member* member, CAtom* atom, PyObject* value )
{
    py_type_fail( "cannot set the value of a signal" );
    return -1;
}


static int
delegate_handler( Member* member, CAtom* atom, PyObject* value )
{
    Member* delegate = member_cast( member->setattr_context );
    return delegate->setattr( atom, value );
}


static int
_mangled_property_handler( Member* member, CAtom* atom, PyObject* value )
{
    char* suffix = (char *)Py23Str_AS_STRING( member->name );
    PyObjectPtr name( Py23Str_FromFormat( "_set_%s", suffix ) );
    if( !name )
        return -1;
    PyObjectPtr callable( PyObject_GetAttr( pyobject_cast( atom ), name.get() ) );
    if( !callable )
    {
        if( PyErr_ExceptionMatches( PyExc_AttributeError ) )
            PyErr_SetString( PyExc_AttributeError, "can't set attribute" );
        return -1;
    }
    PyObjectPtr args( PyTuple_New( 1 ) );
    if( !args )
        return -1;
    PyTuple_SET_ITEM( args.get(), 0, newref( value ) );
    PyObjectPtr ok( PyObject_Call( callable.get(), args.get(), 0 ) );
    if( !ok )
        return -1;
    return 0;
}


static int
property_handler( Member* member, CAtom* atom, PyObject* value )
{
    if( member->setattr_context != Py_None )
    {
        PyObjectPtr args( PyTuple_New( 2 ) );
        if( !args )
            return -1;
        PyTuple_SET_ITEM( args.get(), 0, newref( pyobject_cast( atom ) ) );
        PyTuple_SET_ITEM( args.get(), 1, newref( pyobject_cast( value ) ) );
        PyObjectPtr ok( PyObject_Call( member->setattr_context, args.get(), 0 ) );
        if( !ok )
            return -1;
        return 0;
    }
    return _mangled_property_handler( member, atom, value );
}


static int
call_object_object_value_handler( Member* member, CAtom* atom, PyObject* value )
{
    PyObjectPtr valueptr( newref( value ) );
    valueptr = member->full_validate( atom, Py_None, valueptr.get() );
    if( !valueptr )
        return -1;
    PyObjectPtr callable( newref( member->setattr_context ) );
    PyTuplePtr args( PyTuple_New( 2 ) );
    if( !args )
        return -1;
    args.initialize( 0, newref( pyobject_cast( atom ) ) );
    args.initialize( 1, valueptr );
    if( !callable( args ) )
        return -1;
    return 0;
}


static int
call_object_object_name_value_handler( Member* member, CAtom* atom, PyObject* value )
{
    PyObjectPtr valueptr( newref( value ) );
    valueptr = member->full_validate( atom, Py_None, valueptr.get() );
    if( !valueptr )
        return -1;
    PyObjectPtr callable( newref( member->setattr_context ) );
    PyTuplePtr args( PyTuple_New( 2 ) );
    if( !args )
        return -1;
    args.initialize( 0, newref( pyobject_cast( atom ) ) );
    args.initialize( 1, newref( member->name ) );
    args.initialize( 2, valueptr );
    if( !callable( args ) )
        return -1;
    return 0;
}


static int
object_method_value_handler( Member* member, CAtom* atom, PyObject* value )
{
    PyObjectPtr valueptr( newref( value ) );
    valueptr = member->full_validate( atom, Py_None, valueptr.get() );
    if( !valueptr )
        return -1;
    PyObjectPtr callable( PyObject_GetAttr( pyobject_cast( atom ), member->setattr_context ) );
    if( !callable )
        return -1;
    PyTuplePtr args( PyTuple_New( 1 ) );
    if( !args )
        return -1;
    args.initialize( 0, valueptr );
    if( !callable( args ) )
        return -1;
    return 0;
}


static int
object_method_name_value_handler( Member* member, CAtom* atom, PyObject* value )
{
    PyObjectPtr valueptr( newref( value ) );
    valueptr = member->full_validate( atom, Py_None, valueptr.get() );
    if( !valueptr )
        return -1;
    PyObjectPtr callable( PyObject_GetAttr( pyobject_cast( atom ), member->setattr_context ) );
    if( !callable )
        return -1;
    PyTuplePtr args( PyTuple_New( 2 ) );
    if( !args )
        return -1;
    args.initialize( 0, newref( member->name ) );
    args.initialize( 1, valueptr );
    if( !callable( args ) )
        return -1;
    return 0;
}


static int
member_method_object_value_handler( Member* member, CAtom* atom, PyObject* value )
{
    PyObjectPtr valueptr( newref( value ) );
    valueptr = member->full_validate( atom, Py_None, valueptr.get() );
    if( !valueptr )
        return -1;
    PyObjectPtr callable( PyObject_GetAttr( pyobject_cast( member ), member->setattr_context ) );
    if( !callable )
        return -1;
    PyTuplePtr args( PyTuple_New( 2 ) );
    if( !args )
        return -1;
    args.initialize( 0, newref( pyobject_cast( atom ) ) );
    args.initialize( 1, valueptr );
    if( !callable( args ) )
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


int
Member::setattr( CAtom* atom, PyObject* value )
{
    if( get_setattr_mode() >= sizeof( handlers ) )
        return no_op_handler( this, atom, value );
    return handlers[ get_setattr_mode() ]( this, atom, value );
}
