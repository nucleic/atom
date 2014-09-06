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
	Py_ssize_t m_index;
	PyObject* m_metadata;
	PyObject* m_default_context;
	PyObject* m_validate_context;
	uint8_t m_default_mode;
	uint8_t m_validate_mode;
	uint8_t m_unused_1;
	uint8_t m_unused_2;

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

	static PyObject* Clone( PyObject* member );

	Py_ssize_t index()
	{
		return m_index;
	}

	void setIndex( Py_ssize_t index )
	{
		m_index = index;
	}

	PyObject* defaultValue( PyObject* atom, PyObject* name );

	PyObject* validate( PyObject* atom, PyObject* name, PyObject* value );
};

} // namespace atom
