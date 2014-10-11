/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include "atom.h"
#include "atommeta.h"
#include "errors.h"
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


#ifdef IS_PY3K

PyModuleDef catom_module = {
	PyModuleDef_HEAD_INIT,
	"catom",                   /* m_name */
	"catom extension module",  /* m_doc */
	-1,                        /* m_size */
	catom_methods,             /* m_methods */
	0,                         /* m_reload */
	0,                         /* m_traverse */
	0,                         /* m_clear */
	0,                         /* m_free */
};

#endif


bool ready_types()
{
	using namespace atom;
	if( !Errors::Ready() )
	{
		return false;
	}
	if( !Signal::Ready() )
	{
		return false;
	}
	if( !BoundSignal::Ready() )
	{
		return false;
	}
	if( !Member::Ready() )
	{
		return false;
	}
	if( !Atom::Ready() )
	{
		return false;
	}
	if( !AtomMeta::Ready() )
	{
		return false;
	}
	if( !MethodWrapper::Ready() )
	{
		return false;
	}
	if( !TypedList::Ready() )
	{
		return false;
	}
	return true;
}


bool add_objects( PyObject* mod )
{
	using namespace atom;
	PyObject* ValidationError = cppy::incref( Errors::ValidationError );
	PyObject* Signal = cppy::incref( pyobject_cast( &Signal::TypeObject ) );
	PyObject* BoundSignal = cppy::incref( pyobject_cast( &BoundSignal::TypeObject ) );
	PyObject* Member = cppy::incref( pyobject_cast( &Member::TypeObject ) );
	PyObject* Atom = cppy::incref( pyobject_cast( &Atom::TypeObject ) );
	PyObject* TypedList = cppy::incref( pyobject_cast( &TypedList::TypeObject ) );
	if( PyModule_AddObject( mod, "ValidationError", ValidationError ) < 0 )
	{
		return false;
	}
	if( PyModule_AddObject( mod, "Signal", Signal ) < 0 )
	{
		return false;
	}
	if( PyModule_AddObject( mod, "BoundSignal", BoundSignal ) < 0 )
	{
		return false;
	}
	if( PyModule_AddObject( mod, "CMember", Member ) < 0 )
	{
		return false;
	}
	if( PyModule_AddObject( mod, "CAtom", Atom ) < 0 )
	{
		return false;
	}
	if( PyModule_AddObject( mod, "TypedList", TypedList ) < 0 )
	{
		return false;
	}
	return true;
}

} // namespace


#ifdef IS_PY3K

PyMODINIT_FUNC PyInit_catom( void )
{
	PyObject* mod = PyModule_Create( &catom_module );
	if( !mod )
	{
		return 0;
	}
	if( !ready_types() )
	{
		return 0;
	}
	if( !add_objects( mod ) )
	{
		return 0;
	}
	return mod;
}

#else

PyMODINIT_FUNC initcatom( void )
{
	PyObject* mod = Py_InitModule3( "catom", catom_methods, "catom extension module" );
	if( !mod )
	{
		return;
	}
	if( !ready_types() )
	{
		return;
	}
	if( !add_objects( mod ) )
	{
		return;
	}
}

#endif
