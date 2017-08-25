/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2017, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once

#include "catom.h"
#include "member.h"


namespace MemberChange
{

PyObject*
created( CAtom* atom, Member* member, PyObject* value );


PyObject*
updated( CAtom* atom, Member* member, PyObject* oldvalue, PyObject* newvalue );


PyObject*
deleted( CAtom* atom, Member* member, PyObject* value );


PyObject*
event( CAtom* atom, Member* member, PyObject* value );


PyObject*
property( CAtom* atom, Member* member, PyObject* oldvalue, PyObject* newvalue );

} // namespace MemberChange


int
import_memberchange();
