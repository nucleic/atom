/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include "atom.h"
#include "member.h"

#include <cppy/cppy.h>
#include <Python.h>


using namespace atom;


PyObject* register_members( PyObject* mod, PyObject* args )
{
	PyObject* type;
	PyObject* members;
	if( !PyArg_ParseTuple( args, "OO", &type, &members ) )
	{
		return 0;
	}
	if( !PyType_Check( type ) )
	{
		return cppy::type_error( type, "type" );
	}
	return Atom::RegisterMembers( reinterpret_cast<PyTypeObject*>( type ), members );
}


PyObject* lookup_members( PyObject* mod, PyObject* type )
{
	if( !PyType_Check( type ) )
	{
		return cppy::type_error( type, "type" );
	}
	cppy::ptr members( Atom::LookupMembers( reinterpret_cast<PyTypeObject*>( type ) ) );
	if( !members )
	{
		return 0;
	}
	return PyDict_Copy( members.get() );
}


static PyMethodDef catom_methods[] = {
	{ "_register_members",
	  ( PyCFunction )register_members,
	  METH_VARARGS,
	  "*private* register the members for a given type object" },
	{ "_lookup_members",
	  ( PyCFunction )lookup_members,
	  METH_O,
	  "*private* lookup the members for a given type object" },
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

	Py_INCREF( &Member::TypeObject );
	Py_INCREF( &Atom::TypeObject );
	Py_INCREF( ValidationError );

	PyModule_AddObject( mod, "CMember", reinterpret_cast<PyObject*>( &Member::TypeObject ) );
	PyModule_AddObject( mod, "CAtom", reinterpret_cast<PyObject*>( &Atom::TypeObject ) );
	PyModule_AddObject( mod, "ValidationError", ValidationError );
}
