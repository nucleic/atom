/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once

#include <Python.h>
#include <utils/stdint.h>


namespace atom
{

// POD struct - all member fields are considered private
struct Member
{
    PyObject_HEAD
    PyObject* m_default_context;
    PyObject* m_validate_context;
    PyObject* m_post_validate_context;
    PyObject* m_post_setattr_context;
    uint32_t m_flags;
    uint8_t m_default_mode;
    uint8_t m_validate_mode;
    uint8_t m_post_validate_mode;
    uint8_t m_post_setattr_mode;

    enum Flag
    {
    };

    enum DefaultMode
    {
        NoDefault,
        DefaultValue,
        DefaultList,
        DefaultDict,
        DefaultFactory,
        DefaultAtomMethod,
        DefaultMemberMethod,
        DefaultLast // sentinel
    };

    enum ValidateMode
    {
        NoValidate,
        ValidateBool,
        ValidateInt,
        ValidateFloat,
        ValidateBytes,
        ValidateStr,
        ValidateUnicode,
        ValidateTuple,
        ValidateList,
        ValidateDict,
        ValidateTyped,
        ValidateInstance,
        ValidateSubclass,
        ValidateEnum,
        ValidateCallable,
        ValidateRange,
        ValidateCoerced,
        ValidateAtomMethod,
        ValidateMemberMethod,
        ValidateLast // sentinel
    };

    enum PostValidateMode
    {
        NoPostValidate,
        PostValidateAtomMethod,
        PostValidateMemberMethod,
        PostValidateNone // sentinel
    };

    enum PostSetAttrMode
    {
        NoPostSetAttr,
        PostSetAttrAtomMethod,
        PostSetAttrMemberMethod,
        PostSetAttrLast // sentinel
    };

    static PyTypeObject TypeObject;

    static bool Ready();

    static bool TypeCheck( PyObject* ob )
    {
        return PyObject_TypeCheck( ob, &TypeObject ) != 0;
    }

    bool testFlag( Flag flag )
    {
        return ( m_flags & static_cast<uint32_t>( flag ) ) != 0;
    }

    bool setFlag( Flag flag, bool on = true )
    {
        if( on )
        {
            m_flags |= static_cast<uint32_t>( flag );
        }
        else
        {
            m_flags &= ~( static_cast<uint32_t>( flag ) );
        }
    }

    PyObject* defaultValue( PyObject* atom, PyObject* name );

    PyObject* validateValue( PyObject* atom, PyObject* name, PyObject* value );

    int postSetAttrValue( PyObject* atom, PyObject* name, PyObject* value );
};

} // namespace atom
