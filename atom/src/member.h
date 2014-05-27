/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once

#include <Python.h>
#include "inttypes.h"


struct CAtom;


int
import_member();


extern PyTypeObject Member_Type;


struct Member
{
	enum Flag
	{
		MemberDefault = 0x1,
		MemberValidate = 0x2,
		MemberPostSetattr = 0x4,
		ObjectDefault = 0x8,
		ObjectValidate = 0x10,
		ObjectPostSetattr = 0x20,
		DefaultFactoryContext = 0x40,  // used by subclasses
	};

	// all fields are considered private
    PyObject_HEAD
    PyObject* metadata;
    uint32_t flags;
};


inline int
Member_Check( PyObject* op )
{
    return PyObject_TypeCheck( op, &Member_Type );
}


inline bool
Member_TestFlag( Member* member, Member::Flag flag )
{
    return ( member->flags & static_cast<uint32_t>( flag ) ) != 0;
}


inline void
Member_SetFlag( Member* member, Member::Flag flag, bool on=true )
{
    if( on )
    {
        member->flags |= static_cast<uint32_t>( flag );
    }
    else
    {
        member->flags &= ~( static_cast<uint32_t>( flag ) );
    }
}


PyObject*  // new ref on success, null on failure
Member_Default( Member* member, CAtom* atom, PyStringObject* name );


PyObject*  // new ref on success, null on failure
Member_Validate( Member* member, CAtom* atom, PyStringObject* name, PyObject* old, PyObject* value );


int  // 0 on success, -1 on failure
Member_PostSetAttr( Member* member, CAtom* atom, PyStringObject* name, PyObject* old, PyObject* value );
