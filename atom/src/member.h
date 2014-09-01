/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once

#include "stdint.h"

#include <Python.h>


namespace atom
{

extern PyObject* ValidationError;


// POD struct - all member fields are considered private
struct Member
{
	PyObject_HEAD
	PyObject* m_metadata;
	PyObject* m_default_context;
	PyObject* m_validate_context;
	uint16_t m_value_index;
	uint8_t m_default_mode;
	uint8_t m_validate_mode;

	enum DefaultMode
	{
		NoDefault,
		DefaultValue,
		DefaultFactory,
		DefaultCallObject,
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
		ValidateTyped,
		ValidateInstance,
		ValidateSubclass,
		ValidateEnum,
		ValidateCallable,
		ValidateRange,
		ValidateCoerced,
		ValidateCallObject,
		ValidateMemberMethod,
		ValidateLast // sentinel
	};

	static PyTypeObject TypeObject;

	static bool Ready();

	static bool TypeCheck( PyObject* ob )
	{
		return PyObject_TypeCheck( ob, &TypeObject ) != 0;
	}

	uint16_t valueIndex()
	{
		return m_value_index;
	}

	void setValueIndex( uint16_t index )
	{
		m_value_index = index;
	}

	PyObject* defaultValue( PyObject* atom, PyObject* name );

	PyObject* validateValue( PyObject* atom, PyObject* name, PyObject* value );
};

} // namespace atom
