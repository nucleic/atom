/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2017, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include "pythonhelpers.h"
#include "behaviors.h"
#include "catom.h"
#include "member.h"
#include "memberchange.h"

using namespace PythonHelpers;


static PyObject*
property_args( CAtom* atom, Member* member, PyObject* oldvalue, PyObject* newvalue )
{
    PyTuplePtr argsptr( PyTuple_New( 1 ) );
    if( !argsptr )
        return 0;
    PyObjectPtr changeptr( MemberChange::property( atom, member, oldvalue, newvalue ) );
    if( !changeptr )
        return 0;
    argsptr.initialize( 0, changeptr );
    return argsptr.release();
}


PyObject*
reset_property( PyObject* mod, PyObject* args )
{
    if( PyTuple_GET_SIZE( args ) != 2 )
        return py_type_fail( "reset_property() takes exactly 2 arguments" );
    PyObject* pymember = PyTuple_GET_ITEM( args, 0 );
    PyObject* pyatom = PyTuple_GET_ITEM( args, 1 );
    if( !Member::TypeCheck( pymember ) )
        return py_expected_type_fail( pymember, "Member" );
    if( !CAtom::TypeCheck( pyatom ) )
        return py_expected_type_fail( pyatom, "CAtom" );
    Member* member = member_cast( pymember );
    CAtom* atom = catom_cast( pyatom );
    if( member->index >= atom->get_slot_count() )
        return py_bad_internal_call( "invalid member index" );
    PyObjectPtr oldptr( atom->get_slot( member->index ) );
    atom->set_slot( member->index, 0 );
    bool has_static = member->has_observers();
    bool has_dynamic = atom->has_observers( member->name );
    if( has_static || has_dynamic )
    {
        if( !oldptr )
            oldptr = newref( Py_None );
        PyObjectPtr newptr( member->getattr( atom ) );
        if( !newptr )
            return 0;
        bool cached = member->get_getattr_mode() == GetAttr::CachedProperty;
        if( !cached || !oldptr.richcompare( newptr, Py_EQ ) )
        {
            PyObjectPtr argsptr( property_args( atom, member, oldptr.get(), newptr.get() ) );
            if( !argsptr )
                return 0;
            if( has_static && !member->notify( atom, argsptr.get(), 0 ) )
                return 0;
            if( has_dynamic && !atom->notify( member->name, argsptr.get(), 0 ) )
                return 0;
        }
    }
    Py_RETURN_NONE;
}
