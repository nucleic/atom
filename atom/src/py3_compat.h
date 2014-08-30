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


#define Py23Str_Check PyUnicode_Check
#define Py23Bytes_Check PyBytes_Check
#define Py23Bytes_AS_STRING PyBytes_AS_STRING
#define Py23Int_Check PyLong_Check
#define Py23Int_FromLong PyLong_FromLong
#define Py23Int_AsLong PyLong_AsLong
#define Py23Number_Int PyNumber_Long
#define Py23Int_AsSsize_t PyLong_AsSsize_t
#define Py23Int_FromSsize_t PyLong_FromSsize_t
