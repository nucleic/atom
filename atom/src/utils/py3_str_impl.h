/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once

#include <Python.h>


#define Py23StrObject PyUnicodeObject
#define Py23Str_Check PyUnicode_Check
#define Py23Str_FromString PyUnicode_FromString


namespace atom
{
	// XXX implement me
} // namespace atom
