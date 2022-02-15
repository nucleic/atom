/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2019, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file LICENSE, distributed with this software.
|----------------------------------------------------------------------------*/
#include <cppy/cppy.h>
#include "behaviors.h"
#include "catom.h"
#include "member.h"
#include "memberchange.h"
#include "utils.h"


namespace atom
{


namespace
{


PyObject*
property_args( CAtom* atom, Member* member, PyObject* oldvalue, PyObject* newvalue )
{
    cppy::ptr args( PyTuple_New( 1 ) );
    if( !args )
    {
        return 0;
    }
    cppy::ptr change( MemberChange::property( atom, member, oldvalue, newvalue ) );
    if( !change )
    {
        return 0;
    }
    PyTuple_SET_ITEM( args.get(), 0, change.release() );
    return args.release();
}


}  // namespace


PyObject*
reset_property( PyObject* mod, PyObject* args )
{
    if( PyTuple_GET_SIZE( args ) != 2 )
    {
        return cppy::type_error( "reset_property() takes exactly 2 arguments" );
    }
    PyObject* pymember = PyTuple_GET_ITEM( args, 0 );
    PyObject* pyatom = PyTuple_GET_ITEM( args, 1 );
    if( !Member::TypeCheck( pymember ) )
    {
        return cppy::type_error( pymember, "Member" );
    }
    if( !CAtom::TypeCheck( pyatom ) )
    {
        return cppy::type_error( pyatom, "CAtom" );
    }
    Member* member = member_cast( pymember );
    CAtom* atom = catom_cast( pyatom );
    if( member->index >= atom->get_slot_count() )
    {
        return cppy::system_error( "invalid member index" );
    }
    cppy::ptr oldptr( atom->get_slot( member->index ) );
    atom->set_slot( member->index, 0 );
    bool has_static = member->has_observers( ChangeType::Property );
    bool has_dynamic = atom->has_observers( member->name );
    if( has_static || has_dynamic )
    {
        if( !oldptr )
        {
            oldptr = cppy::incref( Py_None );
        }
        cppy::ptr newptr( member->getattr( atom ) );
        if( !newptr )
        {
            return 0;
        }
        bool cached = member->get_getattr_mode() == GetAttr::CachedProperty;
        if( !cached || !utils::safe_richcompare( oldptr, newptr, Py_EQ ) )
        {
            cppy::ptr argsptr( property_args( atom, member, oldptr.get(), newptr.get() ) );
            if( !argsptr )
            {
                return 0;
            }
            if( has_static && !member->notify( atom, argsptr.get(), 0, ChangeType::Property ) )
            {
                return 0;
            }
            if( has_dynamic && !atom->notify( member->name, argsptr.get(), 0, ChangeType::Property ) )
            {
                return 0;
            }
        }
    }
    Py_RETURN_NONE;
}


}  // namespace atom
