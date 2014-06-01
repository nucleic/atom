/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include <Python.h>
#include "catom.h"
#include "class_map.h"
#include "member.h"

#include "ignoredwarnings.h"


static PyMethodDef catom_methods[] = {
    {0} // Sentinel
};


PyMODINIT_FUNC initcatom( void )
{
    PyObject* mod = Py_InitModule( "catom", catom_methods );
    if( !mod )
    {
        return;
    }
    if( import_member() < 0 )
    {
        return;
    }
    if( import_class_map() < 0 )
    {
        return;
    }
    if( import_catom() < 0 )
    {
        return;
    }

    Py_INCREF( &Member_Type );
    Py_INCREF( &ClassMap_Type );
    Py_INCREF( &CAtom_Type );

    PyModule_AddObject( mod, "Member", ( PyObject* )&Member_Type );
    PyModule_AddObject( mod, "ClassMap", ( PyObject* )&ClassMap_Type );
    PyModule_AddObject( mod, "CAtom", ( PyObject* )&CAtom_Type );
}
