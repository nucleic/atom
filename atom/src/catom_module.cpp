/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include <Python.h>
#include <core/atom.h>
#include <core/class_map.h>
#include <core/member.h>
#include <core/validator.h>


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
    if( !Validator::Ready() )
    {
        return;
    }

    Py_INCREF( &Member::TypeObject );
    Py_INCREF( &ClassMap::TypeObject );
    Py_INCREF( &Atom::TypeObject );
    Py_INCREF( &Validator::TypeObject );

    PyModule_AddObject(
        mod, "Member", reinterpret_cast<PyObject*>( &Member::TypeObject ) );
    PyModule_AddObject(
        mod, "ClassMap", reinterpret_cast<PyObject*>( &ClassMap::TypeObject ) );
    PyModule_AddObject(
        mod, "Atom", reinterpret_cast<PyObject*>( &Atom::TypeObject ) );
    PyModule_AddObject(
        mod, "Validator", reinterpret_cast<PyObject*>( &Validator::TypeObject ) );

    PyModule_AddIntConstant( mod, "Validate_Bool", Validate::Bool );
    PyModule_AddIntConstant( mod, "Validate_Int", Validate::Int );
    PyModule_AddIntConstant( mod, "Validate_Float", Validate::Float );
    PyModule_AddIntConstant( mod, "Validate_Bytes", Validate::Bytes );
    PyModule_AddIntConstant( mod, "Validate_Str", Validate::Str );
    PyModule_AddIntConstant( mod, "Validate_Unicode", Validate::Unicode );
    PyModule_AddIntConstant( mod, "Validate_Tuple", Validate::Tuple );
    PyModule_AddIntConstant( mod, "Validate_List", Validate::List );
    PyModule_AddIntConstant( mod, "Validate_Instance", Validate::Instance );
    PyModule_AddIntConstant( mod, "Validate_Typed", Validate::Typed );
    PyModule_AddIntConstant( mod, "Validate_Subclass", Validate::Subclass );
    PyModule_AddIntConstant( mod, "Validate_Enum", Validate::Enum );
    PyModule_AddIntConstant( mod, "Validate_Callable", Validate::Callable );
    PyModule_AddIntConstant( mod, "Validate_Range", Validate::Range );
    PyModule_AddIntConstant( mod, "Validate_Coerced", Validate::Coerced );
}
