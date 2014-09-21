/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include "atom.h"
#include "atommeta.h"
#include "member.h"
#include "methodwrapper.h"
#include "signal.h"
#include "typedlist.h"

#include <cppy/cppy.h>
#include <Python.h>


#define pyobject_cast( o ) reinterpret_cast<PyObject*>( o )


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
	if( !MethodWrapper::Ready() )
	{
		return;
	}
	if( !TypedList::Ready() )
	{
		return;
	}
	PyModule_AddObject( mod, "Signal",
		cppy::incref( pyobject_cast( &Signal::TypeObject ) ) );
	PyModule_AddObject( mod, "BoundSignal",
		cppy::incref( pyobject_cast( &BoundSignal::TypeObject ) ) );
	PyModule_AddObject( mod, "CMember",
		cppy::incref( pyobject_cast( &Member::TypeObject ) ) );
	PyModule_AddObject( mod, "CAtom",
		cppy::incref( pyobject_cast( &Atom::TypeObject ) ) );
	PyModule_AddObject( mod, "CTypedList",
		cppy::incref( pyobject_cast( &TypedList::TypeObject ) ) );
}
