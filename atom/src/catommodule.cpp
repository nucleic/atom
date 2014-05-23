/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include <Python.h>
#include "catom.h"
#include "descriptor.h"
#include "member.h"

#include "ignoredwarnings.h"


static PyMethodDef
catom_methods[] = {
    { 0 } // Sentinel
};


PyMODINIT_FUNC
initcatom( void )
{
    PyObject* mod = Py_InitModule( "catom", catom_methods );
    if( !mod )
        return;
    if( import_member() < 0 )
        return;
    if( import_descriptor() < 0 )
        return;
    if( import_catom() < 0 )
        return;

    Py_INCREF( &Member_Type );
    Py_INCREF( &Descriptor_Type );
    Py_INCREF( &CAtom_Type );

    PyModule_AddObject( mod, "Member", reinterpret_cast<PyObject*>( &Member_Type ) );
    PyModule_AddObject( mod, "Descriptor", reinterpret_cast<PyObject*>( &Descriptor_Type ) );
    PyModule_AddObject( mod, "CAtom", reinterpret_cast<PyObject*>( &CAtom_Type ) );
}
