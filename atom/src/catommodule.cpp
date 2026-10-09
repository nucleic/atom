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
	if( PyModule_AddObjectRef( mod, "atomlist", atom_list.get() ) < 0 )  // LCOV_EXCL_BR_LINE
	{
		return false;  // LCOV_EXCL_LINE (failed type addition to module)
	}

    // atomclist
    cppy::ptr atom_clist( pyobject_cast( AtomCList::TypeObject ) );
	if( PyModule_AddObjectRef( mod, "atomclist", atom_clist.get() ) < 0 )  // LCOV_EXCL_BR_LINE
	{
		return false;  // LCOV_EXCL_LINE (failed type addition to module)
	}

    // atomdict
    cppy::ptr atom_dict( pyobject_cast( AtomDict::TypeObject ) );
	if( PyModule_AddObjectRef( mod, "atomdict", atom_dict.get() ) < 0 )  // LCOV_EXCL_BR_LINE
	{
		return false;  // LCOV_EXCL_LINE (failed type addition to module)
	}

    // defaultatomdict
    cppy::ptr defaultatom_dict( pyobject_cast( DefaultAtomDict::TypeObject ) );
	if( PyModule_AddObjectRef( mod, "defaultatomdict", defaultatom_dict.get() ) < 0 )  // LCOV_EXCL_BR_LINE
	{
		return false;  // LCOV_EXCL_LINE (failed type addition to module)
	}

    // atomset
    cppy::ptr atom_set( pyobject_cast( AtomSet::TypeObject ) );
	if( PyModule_AddObjectRef( mod, "atomset", atom_set.get() ) < 0 )  // LCOV_EXCL_BR_LINE
	{
		return false;  // LCOV_EXCL_LINE (failed type addition to module)
	}

    // atomref
    cppy::ptr atom_ref( pyobject_cast( AtomRef::TypeObject ) );
	if( PyModule_AddObjectRef( mod, "atomref", atom_ref.get() ) < 0 )  // LCOV_EXCL_BR_LINE
	{
		return false;  // LCOV_EXCL_LINE (failed type addition to module)
	}

    // Member
    cppy::ptr member( pyobject_cast( Member::TypeObject ) );
	if( PyModule_AddObjectRef( mod, "Member", member.get() ) < 0 )  // LCOV_EXCL_BR_LINE
	{
		return false;  // LCOV_EXCL_LINE (failed type addition to module)
	}

    // CAtom
    cppy::ptr catom( pyobject_cast( CAtom::TypeObject ) );
	if( PyModule_AddObjectRef( mod, "CAtom", catom.get() ) < 0 )  // LCOV_EXCL_BR_LINE
	{
		return false;  // LCOV_EXCL_LINE (failed type addition to module)
	}

    if (PyModule_AddObjectRef( mod, "GetAttr", PyGetAttr ) < 0)
        return false;
    if (PyModule_AddObjectRef( mod, "SetAttr", PySetAttr ) < 0)
        return false;
    if (PyModule_AddObjectRef( mod, "DelAttr", PyDelAttr ) < 0)
        return false;
    if (PyModule_AddObjectRef( mod, "PostGetAttr", PyPostGetAttr ) < 0)
        return false;
    if (PyModule_AddObjectRef( mod, "PostSetAttr", PyPostSetAttr ) < 0)
        return false;
    if (PyModule_AddObjectRef( mod, "DefaultValue", PyDefaultValue ) < 0)
        return false;
    if (PyModule_AddObjectRef( mod, "Validate", PyValidate ) < 0)
        return false;
    if (PyModule_AddObjectRef( mod, "PostValidate", PyPostValidate ) < 0)
        return false;
    if (PyModule_AddObjectRef( mod, "GetState", PyGetState ) < 0)
        return false;
    if (PyModule_AddObjectRef( mod, "ChangeType", PyChangeType ) < 0)
        return false;

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
