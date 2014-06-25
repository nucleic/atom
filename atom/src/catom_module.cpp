/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include <atom.h>
#include <atom_list.h>
#include <class_map.h>
#include <member.h>

#include <Python.h>


using namespace atom;


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
    if( !Member::Ready() )
    {
        return;
    }
    if( !ClassMap::Ready() )
    {
        return;
    }
    if( !Atom::Ready() )
    {
        return;
    }
    if( !AtomList::Ready() )
    {
        return;
    }

    Py_INCREF( &Member::TypeObject );
    Py_INCREF( &ClassMap::TypeObject );
    Py_INCREF( &Atom::TypeObject );
    Py_INCREF( ValidationError );

    PyModule_AddObject(
        mod, "CMember", reinterpret_cast<PyObject*>( &Member::TypeObject ) );
    PyModule_AddObject(
        mod, "ClassMap", reinterpret_cast<PyObject*>( &ClassMap::TypeObject ) );
    PyModule_AddObject(
        mod, "CAtom", reinterpret_cast<PyObject*>( &Atom::TypeObject ) );
    PyModule_AddObject( mod, "ValidationError", ValidationError );
}
