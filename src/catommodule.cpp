/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic
|
| Distributed under the terms of the BSD 3-Clause License.
|
| The full license is in the file LICENSE, distributed with this software.
|----------------------------------------------------------------------------*/
#include "atom.h"
#include "errors.h"
#include "member.h"
#include "memberchange.h"
#include "methodwrapper.h"
#include "signal.h"
#include "typeddict.h"
#include "typedlist.h"
#include "typedset.h"

#include <cppy/cppy.h>
#include <Python.h>


#define atom_cast( o ) reinterpret_cast<Atom*>( o )
#define pyobject_cast( o ) reinterpret_cast<PyObject*>( o )
#define signal_cast( o ) reinterpret_cast<Signal*>( o )


namespace
{

using namespace atom;


PyObject* Atom_connect( PyObject* mod, PyObject* args )
{
	PyObject* pyo;
	PyObject* sig;
	PyObject* callback;
	if( !PyArg_UnpackTuple( args, "connect", 3, 3, &pyo, &sig, &callback ) )
	{
		return 0;
	}
	if( !Atom::TypeCheck( pyo ) )
	{
		return cppy::type_error( pyo, "Atom" );
	}
	if( !Signal::TypeCheck( sig ) )
	{
		return cppy::type_error( sig, "Signal" );
	}
	if( !PyCallable_Check( callback ) )
	{
		return cppy::type_error( callback, "callable" );
	}
	if( !Atom::Connect( atom_cast( pyo ), signal_cast( sig ), callback ) )
	{
		return 0;
	}
	return cppy::incref( Py_None );
}


PyObject* Atom_disconnect( PyObject* mod, PyObject* args )
{
	PyObject* pyo;
	PyObject* sig = 0;
	PyObject* callback = 0;
	if( !PyArg_UnpackTuple( args, "disconnect", 1, 3, &pyo, &sig, &callback ) )
	{
		return 0;
	}
	if( !Atom::TypeCheck( pyo ) )
	{
		return cppy::type_error( pyo, "Atom" );
	}
	if( sig && !Signal::TypeCheck( sig ) )
	{
		return cppy::type_error( sig, "Signal" );
	}
	if( callback && !PyCallable_Check( callback ) )
	{
		return cppy::type_error( callback, "callable" );
	}
	if( !sig )
	{
		Atom::Disconnect( atom_cast( pyo ) );
	}
	else if( !callback )
	{
		Atom::Disconnect( atom_cast( pyo ), signal_cast( sig ) );
	}
	else
	{
		Atom::Disconnect( atom_cast( pyo ), signal_cast( sig ), callback );
	}
	return cppy::incref( Py_None );
}


PyObject* Atom_emit( PyObject* mod, PyObject* args, PyObject* kwargs )
{
	Py_ssize_t count = PyTuple_GET_SIZE( args );
	if( count < 2 )
	{
		return cppy::type_error( "emit() takes at least 2 arguments" );
	}
	PyObject* pyo = PyTuple_GET_ITEM( args, 0 );
	PyObject* sig = PyTuple_GET_ITEM( args, 1 );
	if( !Atom::TypeCheck( pyo ) )
	{
		return cppy::type_error( pyo, "Atom" );
	}
	if( !Signal::TypeCheck( sig ) )
	{
		return cppy::type_error( sig, "Signal" );
	}
	cppy::ptr rest( PyTuple_GetSlice( args, 2, count ) );
	if( !rest )
	{
		return 0;
	}
	Atom::Emit( atom_cast( pyo ), signal_cast( sig ), rest.get(), kwargs );
	return cppy::incref( Py_None );
}


PyObject* Atom_sender( PyObject* mod, PyObject* args )
{
	return Atom::Sender();
}


PyMethodDef catom_methods[] = {
	{ "connect",
		( PyCFunction )Atom_connect,
		METH_VARARGS,
		"connect(atom, signal, callback) connect a signal to a callback" },
	{ "disconnect",
		( PyCFunction )Atom_disconnect,
		METH_VARARGS,
		"disconnect(atom[, signal[, callback]) disconnect a signal from a callback" },
	{ "emit",
		( PyCFunction )Atom_emit,
		METH_VARARGS | METH_KEYWORDS,
		"emit(atom, signal, *args, **kwargs) emit a signal with the given arguments" },
	{ "sender",
		( PyCFunction )Atom_sender,
		METH_NOARGS,
		"sender() get the object emitting the current signal" },
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
	if( !MemberChange::Ready() )
	{
		return false;
	}
	if( !Atom::Ready() )
	{
		return false;
	}
	if( !MethodWrapper::Ready() )
	{
		return false;
	}
	if( !TypedDict::Ready() )
	{
		return false;
	}
	if( !TypedList::Ready() )
	{
		return false;
	}
	if( !TypedSet::Ready() )
	{
		return false;
	}
	return true;
}


bool add_objects( PyObject* mod )
{
	PyObject* ValidationError = cppy::incref( Errors::ValidationError );
	PyObject* Signal = cppy::incref( pyobject_cast( &Signal::TypeObject ) );
	PyObject* BoundSignal = cppy::incref( pyobject_cast( &BoundSignal::TypeObject ) );
	PyObject* Member = cppy::incref( pyobject_cast( &Member::TypeObject ) );
	PyObject* Atom = cppy::incref( pyobject_cast( &Atom::TypeObject ) );
	PyObject* TypedDict = cppy::incref( pyobject_cast( &TypedDict::TypeObject ) );
	PyObject* TypedList = cppy::incref( pyobject_cast( &TypedList::TypeObject ) );
	PyObject* TypedSet = cppy::incref( pyobject_cast( &TypedSet::TypeObject ) );
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
	if( PyModule_AddObject( mod, "TypedDict", TypedDict ) < 0 )
	{
		return false;
	}
	if( PyModule_AddObject( mod, "TypedList", TypedList ) < 0 )
	{
		return false;
	}
	if( PyModule_AddObject( mod, "TypedSet", TypedSet ) < 0 )
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
