/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once

#include <cppy/cppy.h>
#include <Python.h>


#define Py23StrObject PyStringObject
#define Py23Str_Check PyString_Check
#define Py23Str_FromString PyString_FromString
#define Py23Bytes_Check PyString_Check
#define Py23Bytes_AS_STRING PyString_AS_STRING
#define Py23Int_Check( ob ) ( PyInt_Check( ob ) || PyLong_Check( ob ) )
#define Py23Int_FromLong PyInt_FromLong
#define Py23Int_AsLong PyInt_AsLong
#define Py23Number_Int PyNumber_Int


inline size_t Py23Str_Hash( Py23StrObject* op )
{
	long hash = op->ob_shash;
	if( hash == -1 )
	{
		hash = PyObject_Hash( reinterpret_cast<PyObject*>( op ) );
	}
	return static_cast<size_t>( hash );
}


inline bool Py23Str_Equal( Py23StrObject* a, Py23StrObject* b )
{
	if( a == b )
	{
		return true;
	}
	if( Py_SIZE( a ) == Py_SIZE( b ) )
	{
		return memcmp( a->ob_sval, b->ob_sval, Py_SIZE( a ) ) == 0;
	}
	return false;
}


inline PyObject* PyName_As23Str( PyObject* name )
{
	if( PyString_Check( name ) )
	{
		return cppy::incref( name );
	}
	if( PyUnicode_Check( name ) )
	{
		// CPython's name conversion policy
		return PyUnicode_AsEncodedString( name, 0, 0 );
	}
	PyErr_Format(
		PyExc_TypeError,
		"attribute name must be string, not '%.200s'",
		Py_TYPE( name )->tp_name );
	return 0;
}
