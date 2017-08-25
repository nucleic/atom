/*-----------------------------------------------------------------------------
| Copyright (c) 2013, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include "enumtypes.h"
#include "packagenaming.h"
#include "py23compat.h"

using namespace PythonHelpers;

#define expand_enum( e ) #e, e


static PyObject* PyIntEnumMeta = 0;
static PyObject* PyIntEnum = 0;


PyObject* PyGetAttr = 0;
PyObject* PySetAttr = 0;
PyObject* PyDelAttr = 0;
PyObject* PyPostGetAttr = 0;
PyObject* PyPostSetAttr = 0;
PyObject* PyDefaultValue = 0;
PyObject* PyValidate = 0;
PyObject* PyPostValidate = 0;


namespace {

template<typename T> inline bool
add_long( PyDictPtr& dict_ptr, const char* name, T value )
{
    PyObjectPtr pyint( Py23Int_FromLong( static_cast<long>( value ) ) );
    if( !pyint )
        return false;
    if( !dict_ptr.set_item( name, pyint ) )
        return false;
    return true;
}


inline PyObject*
make_enum( const char* name, PyDictPtr& dict_ptr )
{
    PyObjectPtr pyname( Py23Str_FromString( name ) );
    if( !pyname )
        return 0;
    PyObjectPtr pybases( PyTuple_Pack( 1, PyIntEnum ) );
    if( !pybases )
        return 0;
    PyDictPtr pydict( PyDict_Copy( dict_ptr.get() ) );
    if( !pydict )
        return 0;
    PyObjectPtr modname( Py23Str_FromString( PACKAGE_PREFIX ) );
    if( !modname )
        return 0;
    if( !pydict.set_item( "__module__", modname ) )
        return 0;
    PyObjectPtr callargs( PyTuple_Pack( 3, pyname.get(), pybases.get(), pydict.get() ) );
    if( !callargs )
        return 0;
    PyObjectPtr enumclass( PyObject_CallObject( PyIntEnumMeta, callargs.get() ) );
    if( !enumclass )
        return 0;
    return enumclass.release();
}

} // namespace


int import_enumtypes()
{
    PyObjectPtr intenum_mod( PyImport_ImportModule( "atom.intenum" ) );
    if( !intenum_mod )
        return -1;
    PyIntEnumMeta = intenum_mod.getattr( "_IntEnumMeta" ).release();
    if( !PyIntEnumMeta )
        return -1;
    PyIntEnum = intenum_mod.getattr( "IntEnum" ).release();
    if( !PyIntEnum )
        return -1;

    {
        using namespace GetAttr;
        PyDictPtr dict_ptr( PyDict_New() );
        if( !dict_ptr )
            return -1;
        add_long( dict_ptr, expand_enum( NoOp ) );
        add_long( dict_ptr, expand_enum( Slot ) );
        add_long( dict_ptr, expand_enum( Event ) );
        add_long( dict_ptr, expand_enum( Signal ) );
        add_long( dict_ptr, expand_enum( Delegate ) );
        add_long( dict_ptr, expand_enum( Property ) );
        add_long( dict_ptr, expand_enum( CachedProperty ) );
        add_long( dict_ptr, expand_enum( CallObject_Object ) );
        add_long( dict_ptr, expand_enum( CallObject_ObjectName ) );
        add_long( dict_ptr, expand_enum( ObjectMethod ) );
        add_long( dict_ptr, expand_enum( ObjectMethod_Name ) );
        add_long( dict_ptr, expand_enum( MemberMethod_Object ) );
        PyGetAttr = make_enum( "GetAttr", dict_ptr );
        if( !PyGetAttr )
            return -1;
    }

    {
        using namespace SetAttr;
        PyDictPtr dict_ptr( PyDict_New() );
        if( !dict_ptr )
            return -1;
        add_long( dict_ptr, expand_enum( NoOp ) );
        add_long( dict_ptr, expand_enum( Slot ) );
        add_long( dict_ptr, expand_enum( Constant ) );
        add_long( dict_ptr, expand_enum( ReadOnly ) );
        add_long( dict_ptr, expand_enum( Event ) );
        add_long( dict_ptr, expand_enum( Signal ) );
        add_long( dict_ptr, expand_enum( Delegate ) );
        add_long( dict_ptr, expand_enum( Property ) );
        add_long( dict_ptr, expand_enum( CallObject_ObjectValue ) );
        add_long( dict_ptr, expand_enum( CallObject_ObjectNameValue ) );
        add_long( dict_ptr, expand_enum( ObjectMethod_Value ) );
        add_long( dict_ptr, expand_enum( ObjectMethod_NameValue ) );
        add_long( dict_ptr, expand_enum( MemberMethod_ObjectValue ) );
        PySetAttr = make_enum( "SetAttr", dict_ptr );
        if( !PySetAttr )
            return -1;
    }

    {
        using namespace DelAttr;
        PyDictPtr dict_ptr( PyDict_New() );
        if( !dict_ptr )
            return -1;
        add_long( dict_ptr, expand_enum( NoOp ) );
        add_long( dict_ptr, expand_enum( Slot ) );
        add_long( dict_ptr, expand_enum( Constant ) );
        add_long( dict_ptr, expand_enum( ReadOnly ) );
        add_long( dict_ptr, expand_enum( Event ) );
        add_long( dict_ptr, expand_enum( Signal ) );
        add_long( dict_ptr, expand_enum( Delegate ) );
        add_long( dict_ptr, expand_enum( Property ) );
        PyDelAttr = make_enum( "DelAttr", dict_ptr );
        if( !PyDelAttr )
            return -1;
    }

    {
        using namespace PostGetAttr;
        PyDictPtr dict_ptr( PyDict_New() );
        if( !dict_ptr )
            return -1;
        add_long( dict_ptr, expand_enum( NoOp ) );
        add_long( dict_ptr, expand_enum( Delegate ) );
        add_long( dict_ptr, expand_enum( ObjectMethod_Value ) );
        add_long( dict_ptr, expand_enum( ObjectMethod_NameValue ) );
        add_long( dict_ptr, expand_enum( MemberMethod_ObjectValue ) );
        PyPostGetAttr = make_enum( "PostGetAttr", dict_ptr );
        if( !PyPostGetAttr )
            return -1;
    }

    {
        using namespace PostSetAttr;
        PyDictPtr dict_ptr( PyDict_New() );
        if( !dict_ptr )
            return -1;
        add_long( dict_ptr, expand_enum( NoOp ) );
        add_long( dict_ptr, expand_enum( Delegate ) );
        add_long( dict_ptr, expand_enum( ObjectMethod_OldNew ) );
        add_long( dict_ptr, expand_enum( ObjectMethod_NameOldNew ) );
        add_long( dict_ptr, expand_enum( MemberMethod_ObjectOldNew ) );
        PyPostSetAttr = make_enum( "PostSetAttr", dict_ptr );
        if( !PyPostSetAttr )
            return -1;
    }

    {
        using namespace DefaultValue;
        PyDictPtr dict_ptr( PyDict_New() );
        if( !dict_ptr )
            return -1;
        add_long( dict_ptr, expand_enum( NoOp ) );
        add_long( dict_ptr, expand_enum( Static ) );
        add_long( dict_ptr, expand_enum( List ) );
        add_long( dict_ptr, expand_enum( Dict ) );
        add_long( dict_ptr, expand_enum( Delegate ) );
        add_long( dict_ptr, expand_enum( CallObject ) );
        add_long( dict_ptr, expand_enum( CallObject_Object ) );
        add_long( dict_ptr, expand_enum( CallObject_ObjectName ) );
        add_long( dict_ptr, expand_enum( ObjectMethod ) );
        add_long( dict_ptr, expand_enum( ObjectMethod_Name ) );
        add_long( dict_ptr, expand_enum( MemberMethod_Object ) );
        PyDefaultValue = make_enum( "DefaultValue", dict_ptr );
        if( !PyDefaultValue )
            return -1;
    }

    {
        using namespace Validate;
        PyDictPtr dict_ptr( PyDict_New() );
        if( !dict_ptr )
            return -1;
        add_long( dict_ptr, expand_enum( NoOp ) );
        add_long( dict_ptr, expand_enum( Bool ) );
        add_long( dict_ptr, expand_enum( Int ) );
        add_long( dict_ptr, expand_enum( IntPromote ) );
        add_long( dict_ptr, expand_enum( Long ) );
        add_long( dict_ptr, expand_enum( LongPromote ) );
        add_long( dict_ptr, expand_enum( Float ) );
        add_long( dict_ptr, expand_enum( FloatPromote ) );
        add_long( dict_ptr, expand_enum( Bytes ) );
        add_long( dict_ptr, expand_enum( BytesPromote ) );
        add_long( dict_ptr, expand_enum( String ) );
        add_long( dict_ptr, expand_enum( StringPromote ) );
        add_long( dict_ptr, expand_enum( Unicode ) );
        add_long( dict_ptr, expand_enum( UnicodePromote ) );
        add_long( dict_ptr, expand_enum( Tuple ) );
        add_long( dict_ptr, expand_enum( List ) );
        add_long( dict_ptr, expand_enum( ContainerList ) );
        add_long( dict_ptr, expand_enum( Dict ) );
        add_long( dict_ptr, expand_enum( Instance ) );
        add_long( dict_ptr, expand_enum( Typed ) );
        add_long( dict_ptr, expand_enum( Subclass ) );
        add_long( dict_ptr, expand_enum( Enum ) );
        add_long( dict_ptr, expand_enum( Callable ) );
        add_long( dict_ptr, expand_enum( FloatRange ) );
        add_long( dict_ptr, expand_enum( Range ) );
        add_long( dict_ptr, expand_enum( Coerced ) );
        add_long( dict_ptr, expand_enum( Delegate ) );
        add_long( dict_ptr, expand_enum( ObjectMethod_OldNew ) );
        add_long( dict_ptr, expand_enum( ObjectMethod_NameOldNew ) );
        add_long( dict_ptr, expand_enum( MemberMethod_ObjectOldNew ) );
        PyValidate = make_enum( "Validate", dict_ptr );
        if( !PyValidate )
            return -1;
    }

    {
        using namespace PostValidate;
        PyDictPtr dict_ptr( PyDict_New() );
        if( !dict_ptr )
            return -1;
        add_long( dict_ptr, expand_enum( NoOp ) );
        add_long( dict_ptr, expand_enum( Delegate ) );
        add_long( dict_ptr, expand_enum( ObjectMethod_OldNew ) );
        add_long( dict_ptr, expand_enum( ObjectMethod_NameOldNew ) );
        add_long( dict_ptr, expand_enum( MemberMethod_ObjectOldNew ) );
        PyPostValidate = make_enum( "PostValidate", dict_ptr );
        if( !PyPostValidate )
            return -1;
    }

    return 0;
}
