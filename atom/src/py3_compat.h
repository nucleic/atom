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


#define Py23StrObject PyUnicodeObject
#define Py23Str_Check PyUnicode_Check
#define Py23Str_FromString PyUnicode_FromString
#define Py23Bytes_Check PyBytes_Check
#define Py23Bytes_AS_STRING PyBytes_AS_STRING
#define Py23Int_Check PyLong_Check
#define Py23Int_FromLong PyLong_FromLong
#define Py23Int_AsLong PyLong_AsLong
#define Py23Number_Int PyNumber_Long


inline size_t Py23Str_Hash( Py23StrObject* op )
{
	// XXX implement me
}


inline bool Py23Str_Equal( Py23StrObject* a, Py23StrObject* b )
{
	// XXX implement me
}


inline PyObject* PyName_As23Str( PyObject* name )
{
	// XXX implement me
}
