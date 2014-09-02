/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include "atom.h"
#include "atom_meta.h"
#include "dispatcher.h"
#include "member.h"

#include <Python.h>


using namespace atom;


PyObject* atom_meta_create_class( PyObject* mod, PyObject* args )
{
	return AtomMeta::CreateClass( args );
}


static PyMethodDef catom_methods[] = {
	{ "_atom_meta_create_class",
	  ( PyCFunction )atom_meta_create_class,
	  METH_VARARGS,
	  "*private* create a new Atom subclass" },
	{ 0 } // sentinel
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
	if( !Atom::Ready() )
	{
		return;
	}
	if( !AtomMeta::Ready() )
	{
		return;
	}
	if( !Dispatcher::Ready() )
	{
		return;
	}
	Py_INCREF( &Member::TypeObject );
	Py_INCREF( &Atom::TypeObject );
	Py_INCREF( &Dispatcher::TypeObject );
	Py_INCREF( ValidationError );
	PyModule_AddObject( mod, "CMember", reinterpret_cast<PyObject*>( &Member::TypeObject ) );
	PyModule_AddObject( mod, "CAtom", reinterpret_cast<PyObject*>( &Atom::TypeObject ) );
	PyModule_AddObject( mod, "Dispatcher", reinterpret_cast<PyObject*>( &Dispatcher::TypeObject ) );
	PyModule_AddObject( mod, "ValidationError", ValidationError );
}
