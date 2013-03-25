/*-----------------------------------------------------------------------------
| Copyright (c) 2013, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include "pythonhelpers.h"
#include "catom.h"
#include "member.h"
#include "memberchange.h"
#include "eventbinder.h"
#include "signalconnector.h"


using namespace PythonHelpers;


static PyMethodDef
catom_methods[] = {
    { 0 } // Sentinel
};


static PyObject*
new_enum_class( const char* name )
{
    PyObjectPtr pyname( PyString_FromString( name ) );
    if( !pyname )
        return 0;
    PyObjectPtr args( PyTuple_New( 0 ) );
    if( !args )
        return 0;
    PyDictPtr kwargs( PyDict_New() );
    if( !kwargs )
        return 0;
    PyObjectPtr modname( PyString_FromString( "catom" ) );
    if( !modname )
        return 0;
    if( !kwargs.set_item( "__module__", modname ) )
        return 0;
    PyObjectPtr callargs( PyTuple_Pack( 3, pyname.get(), args.get(), kwargs.get() ) );
    if( !callargs )
        return 0;
    PyObjectPtr newclass( PyObject_CallObject( pyobject_cast( &PyType_Type ), callargs.get() ) );
    if( !newclass )
        return 0;
    // TODO make these enums more flexible
    pytype_cast( newclass.get() )->tp_new = 0;
    return newclass.release();
}


static int
add_enum( PyObject* cls, const char* name, long value )
{
    PyObjectPtr pyint( PyInt_FromLong( value ) );
    if( !pyint )
        return -1;
    return PyObject_SetAttrString( cls, name, pyint.get() );
}


PyMODINIT_FUNC
initcatom( void )
{
    PyObject* mod = Py_InitModule( "catom", catom_methods );
    if( !mod )
        return;
    if( import_member() < 0 )
        return;
    if( import_memberchange() < 0 )
        return;
    if( import_catom() < 0 )
        return;
    if( import_eventbinder() < 0 )
        return;
    if( import_signalconnector() < 0 )
        return;

    Py_INCREF( &Member_Type );
    Py_INCREF( &CAtom_Type );
    PyModule_AddObject( mod, "Member", reinterpret_cast<PyObject*>( &Member_Type ) );
    PyModule_AddObject( mod, "CAtom", reinterpret_cast<PyObject*>( &CAtom_Type ) );

    PyObject* PyGetAttr = new_enum_class( "GetAttr" );
    if( !PyGetAttr )
        return;
    PyObject* PySetAttr = new_enum_class( "SetAttr" );
    if( !PySetAttr )
        return;
    PyObject* PyDelAttr = new_enum_class( "DelAttr" );
    if( !PyDelAttr )
        return;
    PyObject* PyPostGetAttr = new_enum_class( "PostGetAttr" );
    if( !PyPostGetAttr )
        return;
    PyObject* PyPostSetAttr = new_enum_class( "PostSetAttr" );
    if( !PyPostSetAttr )
        return;
    PyObject* PyDefaultValue = new_enum_class( "DefaultValue" );
    if( !PyDefaultValue )
        return;
    PyObject* PyValidate = new_enum_class( "Validate" );
    if( !PyValidate )
        return;
    PyObject* PyPostValidate = new_enum_class( "PostValidate" );
    if( !PyPostValidate )
        return;

    #define AddEnum( cls, e ) \
        do { \
            if( add_enum( cls, #e, e ) < 0 ) \
                return; \
        } while( 0 )

    {
        using namespace GetAttr;
        AddEnum( PyGetAttr, NoOp );
        AddEnum( PyGetAttr, Slot );
        AddEnum( PyGetAttr, Event );
        AddEnum( PyGetAttr, Signal );
        AddEnum( PyGetAttr, Delegate );
        AddEnum( PyGetAttr, CallObject_Object );
        AddEnum( PyGetAttr, CallObject_ObjectName );
        AddEnum( PyGetAttr, ObjectMethod );
        AddEnum( PyGetAttr, ObjectMethod_Name );
        AddEnum( PyGetAttr, MemberMethod_Object );
    }

    {
        using namespace SetAttr;
        AddEnum( PySetAttr, NoOp );
        AddEnum( PySetAttr, Slot );
        AddEnum( PySetAttr, Constant );
        AddEnum( PySetAttr, ReadOnly );
        AddEnum( PySetAttr, Event );
        AddEnum( PySetAttr, Signal );
        AddEnum( PySetAttr, Delegate );
        AddEnum( PySetAttr, CallObject_ObjectValue );
        AddEnum( PySetAttr, CallObject_ObjectNameValue );
        AddEnum( PySetAttr, ObjectMethod_Value );
        AddEnum( PySetAttr, ObjectMethod_NameValue );
        AddEnum( PySetAttr, MemberMethod_ObjectValue );
    }

    {
        using namespace DelAttr;
        AddEnum( PyDelAttr, NoOp );
        AddEnum( PyDelAttr, Slot );
        AddEnum( PyDelAttr, Constant );
        AddEnum( PyDelAttr, ReadOnly );
        AddEnum( PyDelAttr, Event );
        AddEnum( PyDelAttr, Signal );
        AddEnum( PyDelAttr, Delegate );
    }

    {
        using namespace PostGetAttr;
        AddEnum( PyPostGetAttr, NoOp );
        AddEnum( PyPostGetAttr, Delegate );
        AddEnum( PyPostGetAttr, ObjectMethod_Value );
        AddEnum( PyPostGetAttr, ObjectMethod_NameValue );
        AddEnum( PyPostGetAttr, MemberMethod_ObjectValue );
    }

    {
        using namespace PostSetAttr;
        AddEnum( PyPostSetAttr, NoOp );
        AddEnum( PyPostSetAttr, Delegate );
        AddEnum( PyPostSetAttr, ObjectMethod_OldNew );
        AddEnum( PyPostSetAttr, ObjectMethod_NameOldNew );
        AddEnum( PyPostSetAttr, MemberMethod_ObjectOldNew );
    }

    {
        using namespace DefaultValue;
        AddEnum( PyDefaultValue, NoOp );
        AddEnum( PyDefaultValue, Static );
        AddEnum( PyDefaultValue, List );
        AddEnum( PyDefaultValue, Dict );
        AddEnum( PyDefaultValue, Delegate );
        AddEnum( PyDefaultValue, CallObject );
        AddEnum( PyDefaultValue, CallObject_Object );
        AddEnum( PyDefaultValue, CallObject_ObjectName );
        AddEnum( PyDefaultValue, ObjectMethod );
        AddEnum( PyDefaultValue, ObjectMethod_Name );
        AddEnum( PyDefaultValue, MemberMethod_Object );
    }

    {
        using namespace Validate;
        AddEnum( PyValidate, NoOp );
        AddEnum( PyValidate, Bool );
        AddEnum( PyValidate, Int );
        AddEnum( PyValidate, Long );
        AddEnum( PyValidate, LongPromote );
        AddEnum( PyValidate, Float );
        AddEnum( PyValidate, FloatPromote );
        AddEnum( PyValidate, Str );
        AddEnum( PyValidate, Unicode );
        AddEnum( PyValidate, UnicodePromote );
        AddEnum( PyValidate, Tuple );
        AddEnum( PyValidate, List );
        AddEnum( PyValidate, ListNoCopy );
        AddEnum( PyValidate, Dict );
        AddEnum( PyValidate, Instance );
        AddEnum( PyValidate, Typed );
        AddEnum( PyValidate, Enum );
        AddEnum( PyValidate, Callable );
        AddEnum( PyValidate, Range );
        AddEnum( PyValidate, Coerced );
        AddEnum( PyValidate, Delegate );
        AddEnum( PyValidate, ObjectMethod_OldNew );
        AddEnum( PyValidate, ObjectMethod_NameOldNew );
        AddEnum( PyValidate, MemberMethod_ObjectOldNew );
    }

    {
        using namespace PostValidate;
        AddEnum( PyPostValidate, NoOp );
        AddEnum( PyPostValidate, Delegate );
        AddEnum( PyPostValidate, ObjectMethod_OldNew );
        AddEnum( PyPostValidate, ObjectMethod_NameOldNew );
        AddEnum( PyPostValidate, MemberMethod_ObjectOldNew );
    }

    PyModule_AddObject( mod, "GetAttr", PyGetAttr );
    PyModule_AddObject( mod, "SetAttr", PySetAttr );
    PyModule_AddObject( mod, "DelAttr", PyDelAttr );
    PyModule_AddObject( mod, "PostGetAttr", PyPostGetAttr );
    PyModule_AddObject( mod, "PostSetAttr", PyPostSetAttr );
    PyModule_AddObject( mod, "DefaultValue", PyDefaultValue );
    PyModule_AddObject( mod, "Validate", PyValidate );
    PyModule_AddObject( mod, "PostValidate", PyPostValidate );
}
