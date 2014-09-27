/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
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
