/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2019, Nucleic Development Team.
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
    if( !AtomList::Ready() )
    {
        return false;
    }
    if( !AtomCList::Ready() )
    {
        return false;
    }
    if( !AtomDict::Ready() )
    {
        return false;
    }
    if( !DefaultAtomDict::Ready() )
    {
        return false;
    }
    if( !AtomSet::Ready() )
    {
        return false;
    }
    if( !AtomRef::Ready() )
    {
        return false;
    }
    if( !Member::Ready() )
    {
        return false;
    }
    if( !CAtom::Ready() )
    {
        return false;
    }
    if( !EventBinder::Ready() )
    {
        return false;
    }
    if( !SignalConnector::Ready() )
    {
        return false;
    }
    return true;
}

bool add_objects( PyObject* mod )
{
	using namespace atom;

    // atomlist
    cppy::ptr atom_list( pyobject_cast( AtomList::TypeObject ) );
	if( PyModule_AddObject( mod, "atomlist", atom_list.get() ) < 0 )
	{
		return false;
	}
    atom_list.release();

    // atomclist
    cppy::ptr atom_clist( pyobject_cast( AtomCList::TypeObject ) );
	if( PyModule_AddObject( mod, "atomclist", atom_clist.get() ) < 0 )
	{
		return false;
	}
    atom_clist.release();

    // atomdict
    cppy::ptr atom_dict( pyobject_cast( AtomDict::TypeObject ) );
	if( PyModule_AddObject( mod, "atomdict", atom_dict.get() ) < 0 )
	{
		return false;
	}
    atom_dict.release();

    // defaultatomdict
    cppy::ptr defaultatom_dict( pyobject_cast( DefaultAtomDict::TypeObject ) );
	if( PyModule_AddObject( mod, "defaultatomdict", defaultatom_dict.get() ) < 0 )
	{
		return false;
	}
    defaultatom_dict.release();

    // atomset
    cppy::ptr atom_set( pyobject_cast( AtomSet::TypeObject ) );
	if( PyModule_AddObject( mod, "atomset", atom_set.get() ) < 0 )
	{
		return false;
	}
    atom_set.release();

    // atomref
    cppy::ptr atom_ref( pyobject_cast( AtomRef::TypeObject ) );
	if( PyModule_AddObject( mod, "atomref", atom_ref.get() ) < 0 )
	{
		return false;
	}
    atom_ref.release();

    // Member
    cppy::ptr member( pyobject_cast( Member::TypeObject ) );
	if( PyModule_AddObject( mod, "Member", member.get() ) < 0 )
	{
		return false;
	}
    member.release();

    // CAtom
    cppy::ptr catom( pyobject_cast( CAtom::TypeObject ) );
	if( PyModule_AddObject( mod, "CAtom", catom.get() ) < 0 )
	{
		return false;
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
    if( !ready_types() )
    {
        return -1;
    }
    if( !atom::init_enumtypes() )
    {
        return -1;
    }
    if( !atom::init_memberchange() )
    {
        return -1;
    }
    if( !atom::init_containerlistchange() )
    {
        return -1;
    }
    if( !add_objects( mod ) )
    {
        return -1;
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
