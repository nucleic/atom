/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once

#include <Python.h>


int
import_static_strings();


namespace StaticStrings
{

extern PyObject* ClassMap;

extern PyObject* Default;
extern PyObject* DefaultPrefix;

extern PyObject* PostSetattr;
extern PyObject* PostSetattrPrefix;

extern PyObject* Validate;
extern PyObject* ValidatePrefix;

}  // namespace StaticStrings
