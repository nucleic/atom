/*-----------------------------------------------------------------------------
| Copyright (c) 2013, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include "memberchange.h"


using namespace PythonHelpers;


namespace MemberChange
{

static PyObject* createstr;
static PyObject* updatestr;
static PyObject* deletestr;
static PyObject* eventstr;
static PyObject* typestr;
static PyObject* objectstr;
static PyObject* namestr;
static PyObject* valuestr;
static PyObject* oldvaluestr;


PyObject*
created( CAtom* atom, Member* member, PyObject* value )
{
    PyDictPtr dict( PyDict_New() );
    if( !dict )
        return 0;
    if( !dict.set_item( typestr, createstr ) )
        return 0;
    if( !dict.set_item( objectstr, pyobject_cast( atom ) ) )
        return 0;
    if( !dict.set_item( namestr, member->name ) )
        return 0;
    if( !dict.set_item( valuestr, value ) )
        return 0;
    return dict.release();
}


PyObject*
updated( CAtom* atom, Member* member, PyObject* oldvalue, PyObject* newvalue )
{
    PyDictPtr dict( PyDict_New() );
    if( !dict )
        return 0;
    if( !dict.set_item( typestr, updatestr ) )
        return 0;
    if( !dict.set_item( objectstr, pyobject_cast( atom ) ) )
        return 0;
    if( !dict.set_item( namestr, member->name ) )
        return 0;
    if( !dict.set_item( oldvaluestr, oldvalue ) )
        return 0;
    if( !dict.set_item( valuestr, newvalue ) )
        return 0;
    return dict.release();
}


PyObject*
deleted( CAtom* atom, Member* member, PyObject* value )
{
    PyDictPtr dict( PyDict_New() );
    if( !dict )
        return 0;
    if( !dict.set_item( typestr, deletestr ) )
        return 0;
    if( !dict.set_item( objectstr, pyobject_cast( atom ) ) )
        return 0;
    if( !dict.set_item( namestr, member->name ) )
        return 0;
    if( !dict.set_item( valuestr, value ) )
        return 0;
    return dict.release();
}


PyObject*
event( CAtom* atom, Member* member, PyObject* value )
{
    PyDictPtr dict( PyDict_New() );
    if( !dict )
        return 0;
    if( !dict.set_item( typestr, eventstr ) )
        return 0;
    if( !dict.set_item( objectstr, pyobject_cast( atom ) ) )
        return 0;
    if( !dict.set_item( namestr, member->name ) )
        return 0;
    if( !dict.set_item( valuestr, value ) )
        return 0;
    return dict.release();
}

} // namespace MemberChange


int
import_memberchange()
{
    static bool alloced = false;
    if( alloced )
        return 0;
    MemberChange::createstr = PyString_InternFromString( "create" );
    if( !MemberChange::createstr )
        return -1;
    MemberChange::updatestr = PyString_InternFromString( "update" );
    if( !MemberChange::updatestr )
        return -1;
    MemberChange::deletestr = PyString_InternFromString( "delete" );
    if( !MemberChange::deletestr )
        return -1;
    MemberChange::eventstr = PyString_InternFromString( "event" );
    if( !MemberChange::eventstr )
        return -1;
    MemberChange::typestr = PyString_InternFromString( "type" );
    if( !MemberChange::typestr )
        return -1;
    MemberChange::objectstr = PyString_InternFromString( "object" );
    if( !MemberChange::objectstr )
        return -1;
    MemberChange::namestr = PyString_InternFromString( "name" );
    if( !MemberChange::namestr )
        return -1;
    MemberChange::valuestr = PyString_InternFromString( "value" );
    if( !MemberChange::valuestr )
        return -1;
    MemberChange::oldvaluestr = PyString_InternFromString( "oldvalue" );
    if( !MemberChange::oldvaluestr )
        return -1;
    alloced = true;
    return 0;
}
