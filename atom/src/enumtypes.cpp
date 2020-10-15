/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2020, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file LICENSE, distributed with this software.
|----------------------------------------------------------------------------*/
#include <cppy/cppy.h>
#include "enumtypes.h"
#include "packagenaming.h"

#define expand_enum( e ) #e, e

namespace atom
{


PyObject* PyGetAttr = 0;
PyObject* PySetAttr = 0;
PyObject* PyDelAttr = 0;
PyObject* PyPostGetAttr = 0;
PyObject* PyPostSetAttr = 0;
PyObject* PyDefaultValue = 0;
PyObject* PyValidate = 0;
PyObject* PyPostValidate = 0;


namespace {


static PyObject* PyIntEnumMeta = 0;
static PyObject* PyIntEnum = 0;


template<typename T> inline bool
add_long( cppy::ptr& dict_ptr, const char* name, T value )
{
    cppy::ptr pyint( PyLong_FromLong( static_cast<long>( value ) ) );
    if( !pyint )
    {
        return false;
    }
    if( PyDict_SetItemString( dict_ptr.get(), name, pyint.get() ) != 0 )
    {
        return false;
    }
    pyint.release(); // Release the reference since the operation succeeded
    return true;
}


inline PyObject*
make_enum( const char* name, cppy::ptr& dict_ptr )
{
    cppy::ptr pyname( PyUnicode_FromString( name ) );
    if( !pyname )
    {
        return 0;
    }
    cppy::ptr pybases( PyTuple_Pack( 1, PyIntEnum ) );
    if( !pybases )
    {
        return 0;
    }
    cppy::ptr pydict( PyDict_Copy( dict_ptr.get() ) );
    if( !pydict )
    {
        return 0;
    }
    cppy::ptr modname( PyUnicode_FromString( PACKAGE_PREFIX ) );
    if( !modname )
    {
        return 0;
    }
    if( PyDict_SetItemString( pydict.get(), "__module__", modname.get() ) != 0 )
    {
        return 0;
    }
    cppy::ptr callargs( PyTuple_Pack( 3, pyname.get(), pybases.get(), pydict.get() ) );
    if( !callargs )
    {
        return 0;
    }
    cppy::ptr enumclass( PyObject_CallObject( PyIntEnumMeta, callargs.get() ) );
    if( !enumclass )
    {
        return 0;
    }
    return enumclass.release();
}

} // namespace


bool init_enumtypes()
{
    cppy::ptr intenum_mod( PyImport_ImportModule( "atom.intenum" ) );
    if( !intenum_mod )
    {
        return false;
    }
    PyIntEnumMeta = intenum_mod.getattr( "_IntEnumMeta" );
    if( !PyIntEnumMeta )
    {
        return false;
    }
    PyIntEnum = intenum_mod.getattr( "IntEnum" );
    if( !PyIntEnum )
    {
        return false;
    }

    {
        using namespace GetAttr;
        cppy::ptr dict_ptr( PyDict_New() );
        if( !dict_ptr )
        {
            return false;  // LCOV_EXCL_LINE
        }
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
        {
               return false;
        }
    }

    {
        using namespace SetAttr;
        cppy::ptr dict_ptr( PyDict_New() );
        if( !dict_ptr )
        {
            return false;  // LCOV_EXCL_LINE
        }
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
        {
               return false;
        }
    }

    {
        using namespace DelAttr;
        cppy::ptr dict_ptr( PyDict_New() );
        if( !dict_ptr )
        {
            return false;  // LCOV_EXCL_LINE
        }
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
        {
               return false;
        }
    }

    {
        using namespace PostGetAttr;
        cppy::ptr dict_ptr( PyDict_New() );
        if( !dict_ptr )
        {
            return false;  // LCOV_EXCL_LINE
        }
        add_long( dict_ptr, expand_enum( NoOp ) );
        add_long( dict_ptr, expand_enum( Delegate ) );
        add_long( dict_ptr, expand_enum( ObjectMethod_Value ) );
        add_long( dict_ptr, expand_enum( ObjectMethod_NameValue ) );
        add_long( dict_ptr, expand_enum( MemberMethod_ObjectValue ) );
        PyPostGetAttr = make_enum( "PostGetAttr", dict_ptr );
        if( !PyPostGetAttr )
        {
               return false;
        }
    }

    {
        using namespace PostSetAttr;
        cppy::ptr dict_ptr( PyDict_New() );
        if( !dict_ptr )
        {
            return false;  // LCOV_EXCL_LINE
        }
        add_long( dict_ptr, expand_enum( NoOp ) );
        add_long( dict_ptr, expand_enum( Delegate ) );
        add_long( dict_ptr, expand_enum( ObjectMethod_OldNew ) );
        add_long( dict_ptr, expand_enum( ObjectMethod_NameOldNew ) );
        add_long( dict_ptr, expand_enum( MemberMethod_ObjectOldNew ) );
        PyPostSetAttr = make_enum( "PostSetAttr", dict_ptr );
        if( !PyPostSetAttr )
        {
               return false;
        }
    }

    {
        using namespace DefaultValue;
        cppy::ptr dict_ptr( PyDict_New() );
        if( !dict_ptr )
        {
            return false;  // LCOV_EXCL_LINE
        }
        add_long( dict_ptr, expand_enum( NoOp ) );
        add_long( dict_ptr, expand_enum( Static ) );
        add_long( dict_ptr, expand_enum( List ) );
        add_long( dict_ptr, expand_enum( Set ) );
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
        {
               return false;
        }
    }

    {
        using namespace Validate;
        cppy::ptr dict_ptr( PyDict_New() );
        if( !dict_ptr )
        {
            return false;  // LCOV_EXCL_LINE
        }
        add_long( dict_ptr, expand_enum( NoOp ) );
        add_long( dict_ptr, expand_enum( Bool ) );
        add_long( dict_ptr, expand_enum( Int ) );
        add_long( dict_ptr, expand_enum( IntPromote ) );
        add_long( dict_ptr, expand_enum( Float ) );
        add_long( dict_ptr, expand_enum( FloatPromote ) );
        add_long( dict_ptr, expand_enum( Bytes ) );
        add_long( dict_ptr, expand_enum( BytesPromote ) );
        add_long( dict_ptr, expand_enum( Str ) );
        add_long( dict_ptr, expand_enum( StrPromote ) );
        add_long( dict_ptr, expand_enum( Tuple ) );
        add_long( dict_ptr, expand_enum( List ) );
        add_long( dict_ptr, expand_enum( ContainerList ) );
        add_long( dict_ptr, expand_enum( Set ) );
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
        {
               return false;
        }
    }

    {
        using namespace PostValidate;
        cppy::ptr dict_ptr( PyDict_New() );
        if( !dict_ptr )
        {
            return false;  // LCOV_EXCL_LINE
        }
        add_long( dict_ptr, expand_enum( NoOp ) );
        add_long( dict_ptr, expand_enum( Delegate ) );
        add_long( dict_ptr, expand_enum( ObjectMethod_OldNew ) );
        add_long( dict_ptr, expand_enum( ObjectMethod_NameOldNew ) );
        add_long( dict_ptr, expand_enum( MemberMethod_ObjectOldNew ) );
        PyPostValidate = make_enum( "PostValidate", dict_ptr );
        if( !PyPostValidate )
        {
            return false;
        }
    }

    return true;
}

}  // namespace atom
