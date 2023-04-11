/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2023, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file LICENSE, distributed with this software.
|----------------------------------------------------------------------------*/
#include <cppy/cppy.h>
#include "enumtypes.h"
#include "observer.h"
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
PyObject* PyGetState = 0;
PyObject* PyChangeType = 0;


namespace {

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
make_enum( cppy::ptr& enum_cls, const char* name, cppy::ptr& dict_ptr )
{
    cppy::ptr pyname( PyUnicode_FromString( name ) );
    if( !pyname )
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
    cppy::ptr kwargs( PyDict_New() );
    if( !kwargs )
    {
        return 0;
    }
    if( PyDict_SetItemString( kwargs.get(), "module", modname.get() ) != 0 )
    {
        return 0;
    }
    cppy::ptr callargs( PyTuple_Pack( 2, pyname.get(), pydict.get() ) );
    if( !callargs )
    {
        return 0;
    }
    cppy::ptr enumclass( enum_cls.call( callargs, kwargs ) );
    if( !enumclass )
    {
        return 0;
    }
    return enumclass.release();
}

} // namespace


bool init_enumtypes()
{
    cppy::ptr intenum_mod( PyImport_ImportModule( "enum" ) );
    if( !intenum_mod )
    {
        return false;
    }
    cppy::ptr enum_cls( intenum_mod.getattr( "IntEnum" ) );
    if( !enum_cls )
    {
        return false;
    }
    cppy::ptr flag_cls( intenum_mod.getattr( "IntFlag" ) );
    if( !flag_cls )
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
        PyGetAttr = make_enum( enum_cls, "GetAttr", dict_ptr );
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
        PySetAttr = make_enum( enum_cls, "SetAttr", dict_ptr );
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
        PyDelAttr = make_enum( enum_cls, "DelAttr", dict_ptr );
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
        PyPostGetAttr = make_enum( enum_cls, "PostGetAttr", dict_ptr );
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
        PyPostSetAttr = make_enum( enum_cls, "PostSetAttr", dict_ptr );
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
        add_long( dict_ptr, expand_enum( DefaultDict ) );
        add_long( dict_ptr, expand_enum( NonOptional ) );
        add_long( dict_ptr, expand_enum( Delegate ) );
        add_long( dict_ptr, expand_enum( CallObject ) );
        add_long( dict_ptr, expand_enum( CallObject_Object ) );
        add_long( dict_ptr, expand_enum( CallObject_ObjectName ) );
        add_long( dict_ptr, expand_enum( ObjectMethod ) );
        add_long( dict_ptr, expand_enum( ObjectMethod_Name ) );
        add_long( dict_ptr, expand_enum( MemberMethod_Object ) );
        PyDefaultValue = make_enum( enum_cls, "DefaultValue", dict_ptr );
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
        add_long( dict_ptr, expand_enum( DefaultDict ) );
        add_long( dict_ptr, expand_enum( OptionalInstance ) );
        add_long( dict_ptr, expand_enum( Instance ) );
        add_long( dict_ptr, expand_enum( OptionalTyped ) );
        add_long( dict_ptr, expand_enum( Typed ) );
        add_long( dict_ptr, expand_enum( Subclass ) );
        add_long( dict_ptr, expand_enum( Enum ) );
        add_long( dict_ptr, expand_enum( Callable ) );
        add_long( dict_ptr, expand_enum( FloatRange ) );
        add_long( dict_ptr, expand_enum( FloatRangePromote ) );
        add_long( dict_ptr, expand_enum( Range ) );
        add_long( dict_ptr, expand_enum( Coerced ) );
        add_long( dict_ptr, expand_enum( Delegate ) );
        add_long( dict_ptr, expand_enum( ObjectMethod_OldNew ) );
        add_long( dict_ptr, expand_enum( ObjectMethod_NameOldNew ) );
        add_long( dict_ptr, expand_enum( MemberMethod_ObjectOldNew ) );
        PyValidate = make_enum( enum_cls, "Validate", dict_ptr );
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
        PyPostValidate = make_enum( enum_cls, "PostValidate", dict_ptr );
        if( !PyPostValidate )
        {
            return false;
        }
    }

    {
        using namespace ChangeType;
        cppy::ptr dict_ptr( PyDict_New() );
        if( !dict_ptr )
        {
            return false;  // LCOV_EXCL_LINE
        }
        add_long( dict_ptr, "CREATE", Create );
        add_long( dict_ptr, "UPDATE", Update );
        add_long( dict_ptr, "DELETE", Delete );
        add_long( dict_ptr, "EVENT",  Event );
        add_long( dict_ptr, "PROPERTY", Property );
        add_long( dict_ptr, "CONTAINER", Container );
        add_long( dict_ptr, "ANY", Any );
        PyChangeType = make_enum( flag_cls, "ChangeType", dict_ptr );
        if( !PyChangeType )
        {
            return false;
        }
    }

    {
        using namespace GetState;
        cppy::ptr dict_ptr( PyDict_New() );
        if( !dict_ptr )
        {
            return false;  // LCOV_EXCL_LINE
        }
        add_long( dict_ptr, expand_enum( Exclude ) );
        add_long( dict_ptr, expand_enum( Include ) );
        add_long( dict_ptr, expand_enum( IncludeNonDefault ) );
        add_long( dict_ptr, expand_enum( Property ) );
        add_long( dict_ptr, expand_enum( ObjectMethod_Name ) );
        add_long( dict_ptr, expand_enum( MemberMethod_Object ) );
        PyGetState = make_enum( enum_cls, "GetState", dict_ptr );
        if( !PyGetState )
        {
            return false;
        }
    }

    return true;
}

}  // namespace atom
