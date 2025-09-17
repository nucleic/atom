/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2025, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file LICENSE, distributed with this software.
|----------------------------------------------------------------------------*/
#include <cppy/cppy.h>
#include "behaviors.h"
#include "catom.h"
#include "member.h"
#include "memberchange.h"
#include "eventbinder.h"
#include "signalconnector.h"
#include "atomref.h"
#include "atomlist.h"
#include "atomset.h"
#include "atomdict.h"
#include "enumtypes.h"
#include "propertyhelper.h"


namespace
{


bool ready_types()
{
    using namespace atom;
    if( !AtomList::Ready() )  // LCOV_EXCL_BR_LINE
    {
        return false;  // LCOV_EXCL_LINE (failed type init)
    }
    if( !AtomCList::Ready() )  // LCOV_EXCL_BR_LINE
    {
        return false;  // LCOV_EXCL_LINE (failed type init)
    }
    if( !AtomDict::Ready() )  // LCOV_EXCL_BR_LINE
    {
        return false;  // LCOV_EXCL_LINE (failed type init)
    }
    if( !DefaultAtomDict::Ready() )  // LCOV_EXCL_BR_LINE
    {
        return false;  // LCOV_EXCL_LINE (failed type init)
    }
    if( !AtomSet::Ready() )  // LCOV_EXCL_BR_LINE
    {
        return false;  // LCOV_EXCL_LINE (failed type init)
    }
    if( !AtomRef::Ready() )  // LCOV_EXCL_BR_LINE
    {
        return false;  // LCOV_EXCL_LINE (failed type init)
    }
    if( !Member::Ready() )  // LCOV_EXCL_BR_LINE
    {
        return false;  // LCOV_EXCL_LINE (failed type init)
    }
    if( !CAtom::Ready() )  // LCOV_EXCL_BR_LINE
    {
        return false;  // LCOV_EXCL_LINE (failed type init)
    }
    if( !EventBinder::Ready() )  // LCOV_EXCL_BR_LINE
    {
        return false;  // LCOV_EXCL_LINE (failed type init)
    }
    if( !SignalConnector::Ready() )  // LCOV_EXCL_BR_LINE
    {
        return false;  // LCOV_EXCL_LINE (failed type init)
    }
    return true;
}

bool add_objects( PyObject* mod )
{
	using namespace atom;

    // atomlist
    cppy::ptr atom_list( pyobject_cast( AtomList::TypeObject ) );
	if( PyModule_AddObject( mod, "atomlist", atom_list.get() ) < 0 )  // LCOV_EXCL_BR_LINE
	{
		return false;  // LCOV_EXCL_LINE (failed type addition to module)
	}
    atom_list.release();

    // atomclist
    cppy::ptr atom_clist( pyobject_cast( AtomCList::TypeObject ) );
	if( PyModule_AddObject( mod, "atomclist", atom_clist.get() ) < 0 )  // LCOV_EXCL_BR_LINE
	{
		return false;  // LCOV_EXCL_LINE (failed type addition to module)
	}
    atom_clist.release();

    // atomdict
    cppy::ptr atom_dict( pyobject_cast( AtomDict::TypeObject ) );
	if( PyModule_AddObject( mod, "atomdict", atom_dict.get() ) < 0 )  // LCOV_EXCL_BR_LINE
	{
		return false;  // LCOV_EXCL_LINE (failed type addition to module)
	}
    atom_dict.release();

    // defaultatomdict
    cppy::ptr defaultatom_dict( pyobject_cast( DefaultAtomDict::TypeObject ) );
	if( PyModule_AddObject( mod, "defaultatomdict", defaultatom_dict.get() ) < 0 )  // LCOV_EXCL_BR_LINE
	{
		return false;  // LCOV_EXCL_LINE (failed type addition to module)
	}
    defaultatom_dict.release();

    // atomset
    cppy::ptr atom_set( pyobject_cast( AtomSet::TypeObject ) );
	if( PyModule_AddObject( mod, "atomset", atom_set.get() ) < 0 )  // LCOV_EXCL_BR_LINE
	{
		return false;  // LCOV_EXCL_LINE (failed type addition to module)
	}
    atom_set.release();

    // atomref
    cppy::ptr atom_ref( pyobject_cast( AtomRef::TypeObject ) );
	if( PyModule_AddObject( mod, "atomref", atom_ref.get() ) < 0 )  // LCOV_EXCL_BR_LINE
	{
		return false;  // LCOV_EXCL_LINE (failed type addition to module)
	}
    atom_ref.release();

    // Member
    cppy::ptr member( pyobject_cast( Member::TypeObject ) );
	if( PyModule_AddObject( mod, "Member", member.get() ) < 0 )  // LCOV_EXCL_BR_LINE
	{
		return false;  // LCOV_EXCL_LINE (failed type addition to module)
	}
    member.release();

    // CAtom
    cppy::ptr catom( pyobject_cast( CAtom::TypeObject ) );
	if( PyModule_AddObject( mod, "CAtom", catom.get() ) < 0 )
	{
		return false;  // LCOV_EXCL_LINE (failed type addition to module)
	}
    catom.release();

    cppy::incref( PyGetAttr );
    cppy::incref( PySetAttr );
    cppy::incref( PyDelAttr );
    cppy::incref( PyPostGetAttr );
    cppy::incref( PyPostSetAttr );
    cppy::incref( PyDefaultValue );
    cppy::incref( PyValidate );
    cppy::incref( PyPostValidate );
    cppy::incref( PyGetState );
    cppy::incref( PyChangeType );
    PyModule_AddObject( mod, "GetAttr", PyGetAttr );
    PyModule_AddObject( mod, "SetAttr", PySetAttr );
    PyModule_AddObject( mod, "DelAttr", PyDelAttr );
    PyModule_AddObject( mod, "PostGetAttr", PyPostGetAttr );
    PyModule_AddObject( mod, "PostSetAttr", PyPostSetAttr );
    PyModule_AddObject( mod, "DefaultValue", PyDefaultValue );
    PyModule_AddObject( mod, "Validate", PyValidate );
    PyModule_AddObject( mod, "PostValidate", PyPostValidate );
    PyModule_AddObject( mod, "GetState", PyGetState );
    PyModule_AddObject( mod, "ChangeType", PyChangeType );

	return true;
}


int
catom_modexec( PyObject *mod )
{
    if( !ready_types() )  // LCOV_EXCL_BR_LINE
    {
        return -1;  // LCOV_EXCL_LINE (failed type creation)
    }
    if( !atom::init_enumtypes() )  // LCOV_EXCL_BR_LINE
    {
        return -1;  // LCOV_EXCL_LINE (failed enum creation)
    }
    if( !atom::init_memberchange() )  // LCOV_EXCL_BR_LINE
    {
        return -1;  // LCOV_EXCL_LINE (failed type creation)
    }
    if( !atom::init_containerlistchange() )  // LCOV_EXCL_BR_LINE
    {
        return -1;  // LCOV_EXCL_LINE (failed type creation)
    }
    if( !add_objects( mod ) )  // LCOV_EXCL_BR_LINE
    {
        return -1;  // LCOV_EXCL_LINE (failed type addition to module)
    }


    return 0;
}


PyMethodDef
catom_methods[] = {
    { "reset_property", ( PyCFunction )atom::reset_property, METH_VARARGS,
      "Reset a Property member. For internal use only!" },
    { 0 } // Sentinel
};


PyModuleDef_Slot catom_slots[] = {
    {Py_mod_exec, reinterpret_cast<void*>( catom_modexec ) },
    {0, NULL}
};


struct PyModuleDef moduledef = {
        PyModuleDef_HEAD_INIT,
        "catom",
        "catom extension module",
        0,
        catom_methods,
        catom_slots,
        NULL,
        NULL,
        NULL
};

}  // namespace


PyMODINIT_FUNC PyInit_catom( void )
{
    return PyModuleDef_Init( &moduledef );
}
