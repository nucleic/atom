/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include "atom.h"
#include "atom_meta.h"
#include "member.h"
#include "signal.h"

#include <Python.h>


namespace
{

PyObject* atom_meta_create_class( PyObject* mod, PyObject* args )
{
	return atom::AtomMeta::CreateClass( args );
}


PyMethodDef catom_methods[] = {
	{ "_atom_meta_create_class",
	  ( PyCFunction )atom_meta_create_class,
	  METH_VARARGS,
	  "*private* create a new Atom subclass" },
	{ 0 } // sentinel
};

} // namespace


PyMODINIT_FUNC initcatom( void )
{
	using namespace atom;
	PyObject* mod = Py_InitModule( "catom", catom_methods );
	if( !mod )
	{
		return;
	}
	if( !Signal::Ready() )
	{
		return;
	}
	if( !BoundSignal::Ready() )
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
	Py_INCREF( &Signal::TypeObject );
	Py_INCREF( &BoundSignal::TypeObject );
	Py_INCREF( &Member::TypeObject );
	Py_INCREF( &Atom::TypeObject );
	Py_INCREF( ValidationError );
	PyModule_AddObject( mod, "Signal", reinterpret_cast<PyObject*>( &Signal::TypeObject ) );
	PyModule_AddObject( mod, "BoundSignal", reinterpret_cast<PyObject*>( &BoundSignal::TypeObject ) );
	PyModule_AddObject( mod, "CMember", reinterpret_cast<PyObject*>( &Member::TypeObject ) );
	PyModule_AddObject( mod, "CAtom", reinterpret_cast<PyObject*>( &Atom::TypeObject ) );
	PyModule_AddObject( mod, "ValidationError", ValidationError );
}
