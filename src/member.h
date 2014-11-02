/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic
|
| Distributed under the terms of the BSD 3-Clause License.
|
| The full license is in the file LICENSE, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once

#include "platstdint.h"

#include <Python.h>


namespace atom
{

// POD struct - all member fields are considered private
struct Member
{
	PyObject_HEAD
	Py_ssize_t m_index;
	PyObject* m_metadata;
	PyObject* m_default_context;
	PyObject* m_validate_context;
	PyObject* m_post_validate_context;
	uint8_t m_default_mode;
	uint8_t m_validate_mode;
	uint8_t m_post_validate_mode;
	uint8_t m_unused_2;

	enum DefaultMode
	{
		NoDefault,
		DefaultValue,
		DefaultFactory,
		DefaultCallObject,
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
		ValidateTyped,
		ValidateInstance,
		ValidateSubclass,
		ValidateEnum,
		ValidateCallable,
		ValidateRange,
		ValidateCoerced,
		ValidateTuple,
		ValidateList,
		ValidateDict,
		ValidateSet,
		ValidateCallObject,
		ValidateAtomMethod,
		ValidateMemberMethod,
		ValidateLast // sentinel
	};

	enum PostValidateMode
	{
		NoPostValidate,
		PostValidateCallObject,
		PostValidateAtomMethod,
		PostValidateMemberMethod,
		PostValidateLast // sentinel
	};

	static PyTypeObject TypeObject;

	static bool Ready();

	static bool TypeCheck( PyObject* ob )
	{
		return PyObject_TypeCheck( ob, &TypeObject ) != 0;
	}

	Py_ssize_t index() { return m_index; }

	PyObject* defaultValue( PyObject* atom, PyObject* name );

	PyObject* validateValue( PyObject* atom, PyObject* name, PyObject* value );

	PyObject* postValidateValue( PyObject* atom, PyObject* name, PyObject* value );
};

} // namespace atom
