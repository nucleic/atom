/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2019, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file LICENSE, distributed with this software.
|----------------------------------------------------------------------------*/
#include <cppy/cppy.h>
#include "memberchange.h"


namespace atom
{

namespace MemberChange
{

static PyObject* createstr;
static PyObject* updatestr;
static PyObject* deletestr;
static PyObject* eventstr;
static PyObject* propertystr;
static PyObject* typestr;
static PyObject* objectstr;
static PyObject* namestr;
static PyObject* valuestr;
static PyObject* oldvaluestr;


PyObject*
created( CAtom* atom, Member* member, PyObject* value )
{
    cppy::ptr dict( PyDict_New() );
    if( !dict )
    {
        return 0;
    }
    if( PyDict_SetItem( dict.get(),  typestr, createstr ) != 0 )
    {
        return 0;
    }
    if( PyDict_SetItem( dict.get(),  objectstr, pyobject_cast( atom ) ) != 0 )
    {
        return 0;
    }
    if( PyDict_SetItem( dict.get(),  namestr, member->name ) != 0 )
    {
        return 0;
    }
    if( PyDict_SetItem( dict.get(),  valuestr, value ) != 0 )
    {
        return 0;
    }
    return dict.release();
}


PyObject*
updated( CAtom* atom, Member* member, PyObject* oldvalue, PyObject* newvalue )
{
    cppy::ptr dict( PyDict_New() );
    if( !dict )
    {
        return 0;
    }
    if( PyDict_SetItem( dict.get(),  typestr, updatestr ) != 0 )
    {
        return 0;
    }
    if( PyDict_SetItem( dict.get(),  objectstr, pyobject_cast( atom ) ) != 0 )
    {
        return 0;
    }
    if( PyDict_SetItem( dict.get(),  namestr, member->name ) != 0 )
    {
        return 0;
    }
    if( PyDict_SetItem( dict.get(),  oldvaluestr, oldvalue ) != 0 )
    {
        return 0;
    }
    if( PyDict_SetItem( dict.get(),  valuestr, newvalue ) != 0 )
    {
        return 0;
    }
    return dict.release();
}


PyObject*
deleted( CAtom* atom, Member* member, PyObject* value )
{
    cppy::ptr dict( PyDict_New() );
    if( !dict )
    {
        return 0;
    }
    if( PyDict_SetItem( dict.get(),  typestr, deletestr ) != 0 )
    {
        return 0;
    }
    if( PyDict_SetItem( dict.get(),  objectstr, pyobject_cast( atom ) ) != 0 )
    {
        return 0;
    }
    if( PyDict_SetItem( dict.get(),  namestr, member->name ) != 0 )
    {
        return 0;
    }
    if( PyDict_SetItem( dict.get(),  valuestr, value ) != 0 )
    {
        return 0;
    }
    return dict.release();
}


PyObject*
event( CAtom* atom, Member* member, PyObject* value )
{
    cppy::ptr dict( PyDict_New() );
    if( !dict )
    {
        return 0;
    }
    if( PyDict_SetItem( dict.get(),  typestr, eventstr ) != 0 )
    {
        return 0;
    }
    if( PyDict_SetItem( dict.get(),  objectstr, pyobject_cast( atom ) ) != 0 )
    {
        return 0;
    }
    if( PyDict_SetItem( dict.get(),  namestr, member->name ) != 0 )
    {
        return 0;
    }
    if( PyDict_SetItem( dict.get(),  valuestr, value ) != 0 )
    {
        return 0;
    }
    return dict.release();
}


PyObject*
property( CAtom* atom, Member* member, PyObject* oldvalue, PyObject* newvalue )
{
    cppy::ptr dict( PyDict_New() );
    if( !dict )
    {
        return 0;
    }
    if( PyDict_SetItem( dict.get(),  typestr, propertystr ) != 0)
    {
        return 0;
    }
    if( PyDict_SetItem( dict.get(),  objectstr, pyobject_cast( atom ) ) != 0 )
    {
        return 0;
    }
    if( PyDict_SetItem( dict.get(),  namestr, member->name ) != 0 )
    {
        return 0;
    }
    if( PyDict_SetItem( dict.get(),  oldvaluestr, oldvalue ) != 0 )
    {
        return 0;
    }
    if( PyDict_SetItem( dict.get(),  valuestr, newvalue ) != 0 )
    {
        return 0;
    }
    return dict.release();
}

} // namespace MemberChange


bool
init_memberchange()
{
    static bool alloced = false;
    if( alloced )
    {
        return true;
    }
    MemberChange::createstr = PyUnicode_InternFromString( "create" );
    if( !MemberChange::createstr )
    {
        return false;
    }
    MemberChange::updatestr = PyUnicode_InternFromString( "update" );
    if( !MemberChange::updatestr )
    {
        return false;
    }
    MemberChange::deletestr = PyUnicode_InternFromString( "delete" );
    if( !MemberChange::deletestr )
    {
        return false;
    }
    MemberChange::eventstr = PyUnicode_InternFromString( "event" );
    if( !MemberChange::eventstr )
    {
        return false;
    }
    MemberChange::propertystr = PyUnicode_InternFromString( "property" );
    if( !MemberChange::propertystr )
    {
        return false;
    }
    MemberChange::typestr = PyUnicode_InternFromString( "type" );
    if( !MemberChange::typestr )
    {
        return false;
    }
    MemberChange::objectstr = PyUnicode_InternFromString( "object" );
    if( !MemberChange::objectstr )
    {
        return false;
    }
    MemberChange::namestr = PyUnicode_InternFromString( "name" );
    if( !MemberChange::namestr )
    {
        return false;
    }
    MemberChange::valuestr = PyUnicode_InternFromString( "value" );
    if( !MemberChange::valuestr )
    {
        return false;
    }
    MemberChange::oldvaluestr = PyUnicode_InternFromString( "oldvalue" );
    if( !MemberChange::oldvaluestr )
    {
        return false;
    }
    alloced = true;
    return true;
}

}  // namespace atom
