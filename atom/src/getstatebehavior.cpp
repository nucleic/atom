/*-----------------------------------------------------------------------------
| Copyright (c) 2023, Atom Development Team.
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
Member::check_context( GetState::Mode mode, PyObject* context )
{
    switch( mode )
    {
        case GetState::ObjectMethod_Name:
        case GetState::MemberMethod_Object:
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
exclude_handler( Member* member, CAtom* atom )
{
    return cppy::incref( Py_False );
}


PyObject*
include_handler( Member* member, CAtom* atom )
{
    return cppy::incref( Py_True );
}


PyObject*
include_non_default_handler( Member* member, CAtom* atom )
{
    if( member->index >= atom->get_slot_count() ) {
        return cppy::attribute_error( pyobject_cast( atom ), (char const *)PyUnicode_AsUTF8( member->name ) );
    }
    cppy::ptr value( atom->get_slot( member->index ) );
    if( value ) {
        return cppy::incref( Py_True );
    }
    else {
        return cppy::incref( Py_False );
    }
}


PyObject*
property_handler( Member* member, CAtom* atom )
{
    // Pickle a property only if the value can be set
    if( member->get_setattr_mode() == SetAttr::Property && member->setattr_context != Py_None ) {
        return cppy::incref( Py_True );
    }
    else {
        return cppy::incref( Py_False );
    }
}


PyObject*
object_method_name_handler( Member* member, CAtom* atom )
{
    cppy::ptr callable( PyObject_GetAttr( pyobject_cast( atom ), member->getstate_context ) );
    if( !callable )
        return 0;
    cppy::ptr args( PyTuple_New( 1 ) );
    if( !args )
        return 0;
    PyTuple_SET_ITEM( args.get(), 0, cppy::incref( member->name ) );
    cppy::ptr result( callable.call( args ) );
    if( !result )
        return 0;
    return result.release();
}


PyObject*
member_method_object_handler( Member* member, CAtom* atom )
{
    cppy::ptr callable( PyObject_GetAttr( pyobject_cast( member ), member->getstate_context ) );
    if( !callable )
        return 0;
    cppy::ptr args( PyTuple_New( 1 ) );
    if( !args )
        return 0;
    PyTuple_SET_ITEM( args.get(), 0, cppy::incref( pyobject_cast( atom ) ) );
    cppy::ptr result( callable.call( args ) );
    if( !result )
        return 0;
    return result.release();
}


typedef PyObject*
( *handler )( Member* member, CAtom* atom );


static handler
handlers[] = {
    include_handler,  // We want the include handler to be the default one
    exclude_handler,
    include_non_default_handler,
    property_handler,
    object_method_name_handler,
    member_method_object_handler
};

}  // namespace


PyObject*
Member::should_getstate( CAtom* atom )
{
    if( get_getstate_mode() >= sizeof( handlers ) )
        return include_handler( this, atom );  // LCOV_EXCL_LINE
    return handlers[ get_getstate_mode() ]( this, atom );
}

}  // namespace atom
