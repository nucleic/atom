/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once

#include <Python.h>


#define Py23Str_Check PyString_Check
#define Py23Bytes_Check PyString_Check
#define Py23Bytes_AS_STRING PyString_AS_STRING
#define Py23Int_Check( ob ) ( PyInt_Check( ob ) || PyLong_Check( ob ) )
#define Py23Int_FromLong PyInt_FromLong
#define Py23Int_AsLong PyInt_AsLong
#define Py23Number_Int PyNumber_Int
#define Py23Int_AsSsize_t PyInt_AsSsize_t
#define Py23Int_FromSsize_t PyInt_FromSsize_t


inline PyObject* Py23Method_New( PyObject* func, PyObject* ob )
{
	return PyMethod_New( func, ob, reinterpret_cast<PyObject*>( ob->ob_type ) );
}
