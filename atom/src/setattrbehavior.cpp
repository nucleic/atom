/*-----------------------------------------------------------------------------
| Copyright (c) 2013, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include "member.h"
#include "pynull.h"


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
            if( !PyString_Check( context ) )
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


static PyObject* createdstr;
static PyObject* updatedstr;
static PyObject* deletedstr;
static PyObject* eventstr;
static PyObject* typestr;
static PyObject* objectstr;
static PyObject* namestr;
static PyObject* valuestr;
static PyObject* oldvaluestr;
static PyObject* newvaluestr;


static bool
make_static_strs()
{
    static bool alloced = false;
    if( alloced )
        return true;
    if( !createdstr )
    {
        createdstr = PyString_InternFromString( "created" );
        if( !createdstr )
            return false;
    }
    if( !updatedstr )
    {
        updatedstr = PyString_InternFromString( "updated" );
        if( !updatedstr )
            return false;
    }
    if( !deletedstr )
    {
        deletedstr = PyString_InternFromString( "deleted" );
        if( !deletedstr )
            return false;
    }
    if( !eventstr )
    {
        eventstr = PyString_InternFromString( "event" );
        if( !eventstr )
            return false;
    }
    if( !typestr )
    {
        typestr = PyString_InternFromString( "type" );
        if( !typestr )
            return false;
    }
    if( !objectstr )
    {
        objectstr = PyString_InternFromString( "object" );
        if( !objectstr )
            return false;
    }
    if( !namestr )
    {
        namestr = PyString_InternFromString( "name" );
        if( !namestr )
            return false;
    }
    if( !valuestr )
    {
        valuestr = PyString_InternFromString( "value" );
        if( !valuestr )
            return false;
    }
    if( !oldvaluestr )
    {
        oldvaluestr = PyString_InternFromString( "oldvalue" );
        if( !oldvaluestr )
            return false;
    }
    if( !newvaluestr )
    {
        newvaluestr = PyString_InternFromString( "newvalue" );
        if( !newvaluestr )
            return false;
    }
    alloced = true;
    return true;
}


static PyObject*
make_change_args( CAtom* atom, PyObject* name, PyObject* oldvalue, PyObject* newvalue )
{
    if( !make_static_strs() )
        return 0;
    PyDictPtr dict( PyDict_New() );
    if( !dict )
        return 0;
    PyTuplePtr args( PyTuple_New( 1 ) );
    if( !args )
        return 0;
    PyObject* changetype;
    if( oldvalue == py_null )
        changetype = createdstr;
    else if( newvalue == py_null )
        changetype = deletedstr;
    else
        changetype = updatedstr;
    if( !dict.set_item( typestr, changetype ) )
        return 0;
    if( !dict.set_item( objectstr, pyobject_cast( atom ) ) )
        return 0;
    if( !dict.set_item( namestr, name ) )
        return 0;
    if( !dict.set_item( oldvaluestr, oldvalue ) )
        return 0;
    if( !dict.set_item( newvaluestr, newvalue ) )
        return 0;
    args.initialize( 0, dict );
    return args.release();
}


static int
slot_handler( Member* member, CAtom* atom, PyObject* value )
{
    if( member->index >= atom->get_slot_count() )
    {
        py_no_attr_fail( pyobject_cast( atom ), PyString_AsString( member->name ) );
        return -1;
    }
    PyObjectPtr oldptr( atom->get_slot( member->index ) );
    PyObjectPtr newptr( xnewref( value != py_null ? value : 0 ) );
    if( oldptr == newptr )
        return 0;
    if( !oldptr )
        oldptr.set( newref( py_null ) );
    if( !newptr )
        newptr.set( newref( py_null ) );
    newptr = member->full_validate( atom, oldptr.get(), newptr.get() );
    if( !newptr )
        return -1;
    atom->set_slot( member->index, newptr != py_null ? newptr.get() : 0 );
    if( member->get_post_setattr_mode() )
    {
        if( member->post_setattr( atom, oldptr.get(), newptr.get() ) < 0 )
            return -1;
    }
    if( oldptr != newptr && atom->get_notifications_enabled() )
    {
        PyObjectPtr argsptr;
        if( member->has_observers() )
        {
            if( oldptr.richcompare( newptr, Py_EQ ) )
                return 0;
            argsptr = make_change_args( atom, member->name, oldptr.get(), newptr.get() );
            if( !argsptr )
                return -1;
            if( !member->notify( atom, argsptr.get(), 0 ) )
                return -1;
        }
        if( atom->has_observers( member->name ) )
        {
            if( !argsptr )
            {
                if( oldptr.richcompare( newptr, Py_EQ ) )
                    return 0;
                argsptr = make_change_args( atom, member->name, oldptr.get(), newptr.get() );
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
        py_no_attr_fail( pyobject_cast( atom ), PyString_AsString( member->name ) );
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
make_event_args( CAtom* atom, PyObject* name, PyObject* value )
{
    if( !make_static_strs() )
        return 0;
    PyDictPtr dict( PyDict_New() );
    if( !dict )
        return 0;
    PyTuplePtr args( PyTuple_New( 1 ) );
    if( !args )
        return 0;
    if( !dict.set_item( typestr, eventstr ) )
        return 0;
    if( !dict.set_item( objectstr, pyobject_cast( atom ) ) )
        return 0;
    if( !dict.set_item( namestr, name ) )
        return 0;
    if( !dict.set_item( valuestr, value ) )
        return 0;
    args.initialize( 0, dict );
    return args.release();
}


static int
event_handler( Member* member, CAtom* atom, PyObject* value )
{
    PyObjectPtr valueptr( member->full_validate( atom, py_null, value ? value : py_null ) );
    if( !valueptr )
        return -1;
    if( atom->get_notifications_enabled() )
    {
        PyObjectPtr argsptr;
        if( member->has_observers() )
        {
            argsptr = make_event_args( atom, member->name, valueptr.get() );
            if( !argsptr )
                return -1;
            if( !member->notify( atom, argsptr.get(), 0 ) )
                return -1;
        }
        if( atom->has_observers( member->name ) )
        {
            if( !argsptr )
            {
                argsptr = make_event_args( atom, member->name, valueptr.get() );
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
    py_type_fail( "cannot assign to a Signal" );
    return -1;
}


static int
delegate_handler( Member* member, CAtom* atom, PyObject* value )
{
    Member* delegate = member_cast( member->setattr_context );
    return delegate->setattr( atom, value );
}


static int
call_object_object_value_handler( Member* member, CAtom* atom, PyObject* value )
{
    PyObjectPtr newptr( newref( value ? value : py_null ) );
    newptr = member->full_validate( atom, py_null, newptr.get() );
    if( !newptr )
        return -1;
    PyObjectPtr callable( newref( member->setattr_context ) );
    PyTuplePtr args( PyTuple_New( 2 ) );
    if( !args )
        return -1;
    args.initialize( 0, newref( pyobject_cast( atom ) ) );
    args.initialize( 1, newptr );
    if( !callable( args ) )
        return -1;
    return 0;
}


static int
call_object_object_name_value_handler( Member* member, CAtom* atom, PyObject* value )
{
    PyObjectPtr newptr( newref( value ? value : py_null ) );
    newptr = member->full_validate( atom, py_null, newptr.get() );
    if( !newptr )
        return -1;
    PyObjectPtr callable( newref( member->setattr_context ) );
    PyTuplePtr args( PyTuple_New( 2 ) );
    if( !args )
        return -1;
    args.initialize( 0, newref( pyobject_cast( atom ) ) );
    args.initialize( 1, newref( member->name ) );
    args.initialize( 2, newptr );
    if( !callable( args ) )
        return -1;
    return 0;
}


static int
object_method_value_handler( Member* member, CAtom* atom, PyObject* value )
{
    PyObjectPtr newptr( newref( value ? value : py_null ) );
    newptr = member->full_validate( atom, py_null, newptr.get() );
    if( !newptr )
        return -1;
    PyObjectPtr callable( PyObject_GetAttr( pyobject_cast( atom ), member->setattr_context ) );
    if( !callable )
        return -1;
    PyTuplePtr args( PyTuple_New( 1 ) );
    if( !args )
        return -1;
    args.initialize( 0, newptr );
    if( !callable( args ) )
        return -1;
    return 0;
}


static int
object_method_name_value_handler( Member* member, CAtom* atom, PyObject* value )
{
    PyObjectPtr newptr( newref( value ? value : py_null ) );
    newptr = member->full_validate( atom, py_null, newptr.get() );
    if( !newptr )
        return -1;
    PyObjectPtr callable( PyObject_GetAttr( pyobject_cast( atom ), member->setattr_context ) );
    if( !callable )
        return -1;
    PyTuplePtr args( PyTuple_New( 2 ) );
    if( !args )
        return -1;
    args.initialize( 0, newref( member->name ) );
    args.initialize( 1, newptr );
    if( !callable( args ) )
        return -1;
    return 0;
}


static int
member_method_object_value_handler( Member* member, CAtom* atom, PyObject* value )
{
    PyObjectPtr newptr( newref( value ? value : py_null ) );
    newptr = member->full_validate( atom, py_null, newptr.get() );
    if( !newptr )
        return -1;
    PyObjectPtr callable( PyObject_GetAttr( pyobject_cast( member ), member->setattr_context ) );
    if( !callable )
        return -1;
    PyTuplePtr args( PyTuple_New( 2 ) );
    if( !args )
        return -1;
    args.initialize( 0, newref( pyobject_cast( atom ) ) );
    args.initialize( 1, newptr );
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
