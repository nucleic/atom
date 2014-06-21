/*-----------------------------------------------------------------------------
| Copyright (c) 2013, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once
#include "catom.h"


extern PyTypeObject AtomRef_Type;


int
import_atomref();


namespace SharedAtomRef
{

void
clear( CAtom* atom );

}  // namespace SharedAtomRef
