/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include <Python.h>
#include "member.h"


int import_value();


extern PyTypeObject Value_Type;


struct Value
{
	Member base;
	PyObject* default_context;
};
