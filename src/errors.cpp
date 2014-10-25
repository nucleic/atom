/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic
|
| Distributed under the terms of the BSD 3-Clause License.
|
| The full license is in the file LICENSE, distributed with this software.
|----------------------------------------------------------------------------*/
#include "errors.h"


namespace atom
{

PyObject* Errors::ValidationError = 0;


bool Errors::Ready()
{
	ValidationError = PyErr_NewException( "atom.catom.ValidationError", 0, 0 );
	return ValidationError != 0;
}

} // namespace atom
