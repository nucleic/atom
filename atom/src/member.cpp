/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include <member.h>

#include <utils/py23compat.h>

#include <cppy/cppy.h>


namespace atom
{

PyObject* ValidationError;


namespace
{

bool check_context( Member::DefaultMode mode, PyObject* context )
{
    switch( mode )
    {
        case Member::DefaultList:
        {
            if( context != Py_None && !PyList_Check( context ) )
            {
                cppy::type_error( context, "list or None" );
                return false;
            }
            break;
        }
        case Member::DefaultDict:
        {
            if( context != Py_None && !PyDict_Check( context ) )
            {
                cppy::type_error( context, "dict or None" );
                return false;
            }
            break;
        }
        case Member::DefaultFactory:
        case Member::DefaultCallObject:
        {
            if( !PyCallable_Check( context ) )
            {
                cppy::type_error( context, "callable" );
                return false;
            }
            break;
        }
        case Member::DefaultAtomMethod:
        case Member::DefaultMemberMethod:
        {
            if( !Py23Str_Check( context ) )
            {
                cppy::type_error( context, "str" );
                return false;
            }
            break;
        }
        default:
        {
            break;
        }
    }
    return true;
}


bool check_context( Member::ValidateMode mode, PyObject* context )
{
    switch( mode )
    {
        case Member::ValidateBool:
        case Member::ValidateInt:
        case Member::ValidateFloat:
        case Member::ValidateBytes:
        case Member::ValidateStr:
        case Member::ValidateUnicode:
        {
            if( !PyBool_Check( context ) )
            {
                cppy::type_error( context, "bool" );
                return false;
            }
            break;
        }
        case Member::ValidateTuple:
        case Member::ValidateList:
        {
            if( context != Py_None && !Member::TypeCheck( context ) )
            {
                cppy::type_error( context, "Member or None" );
                return false;
            }
            break;
        }
        case Member::ValidateDict:
        {
            if( !PyTuple_Check( context ) || PyTuple_GET_SIZE( context ) != 2 )
            {
                cppy::type_error( context, "2-tuple of Member or None" );
                return false;
            }
            PyObject* key = PyTuple_GET_ITEM( context, 0 );
            PyObject* val = PyTuple_GET_ITEM( context, 1 );
            if( ( key != Py_None && !Member::TypeCheck( key ) ) ||
                ( val != Py_None && !Member::TypeCheck( val ) ) )
            {
                cppy::type_error( context, "2-tuple of Member or None" );
                return false;
            }
            break;
        }
        case Member::ValidateTyped:
        {
            if( !PyType_Check( context ) )
            {
                cppy::type_error( context, "type" );
                return false;
            }
            break;
        }
        // XXX validate a valid isinstance context for Instance?
        // XXX validate a valid issubclass context for Subclass?
        case Member::ValidateEnum:
        {
            if( !PySequence_Check( context ) )
            {
                cppy::type_error( context, "sequence" );
                return false;
            }
            break;
        }
        case Member::ValidateRange:
        {
            if( !PyTuple_Check( context ) || PyTuple_GET_SIZE( context ) != 3 )
            {
                cppy::type_error( context, "3-tuple of (low, high, kind)" );
                return false;
            }
            // XXX validate valid isinstance context for range kind?
            break;
        }
        case Member::ValidateCoerced:
        {
            if( !PyTuple_Check( context ) || PyTuple_GET_SIZE( context ) != 2 )
            {
                cppy::type_error( context, "2-tuple of (kind, callable)" );
                return false;
            }
            if( !PyCallable_Check( PyTuple_GET_ITEM( context, 1 ) ) )
            {
                cppy::type_error( context, "2-tuple of (kind, callable)" );
                return false;
            }
            // XXX validate valid isinstance context for kind?
            break;
        }
        case Member::ValidateCallObject:
        {
            if( !PyCallable_Check( context ) )
            {
                cppy::type_error( context, "callable" );
                return false;
            }
            break;
        }
        case Member::ValidateAtomMethod:
        case Member::ValidateMemberMethod:
        {
            if( !Py23Str_Check( context ) )
            {
                cppy::type_error( context, "str" );
                return false;
            }
            break;
        }
        default:
        {
            break;
        }
    }
    return true;
}


bool check_context( Member::PostValidateMode mode, PyObject* context )
{
    switch( mode )
    {
        case Member::PostValidateCallObject:
        {
            if( !PyCallable_Check( context ) )
            {
                cppy::type_error( context, "callable" );
                return false;
            }
            break;
        }
        case Member::PostValidateAtomMethod:
        case Member::PostValidateMemberMethod:
        {
            if( !Py23Str_Check( context ) )
            {
                cppy::type_error( context, "str" );
                return false;
            }
            break;
        }
        default:
        {
            break;
        }
    }
    return true;
}


bool check_context( Member::PostSetAttrMode mode, PyObject* context )
{
    switch( mode )
    {
        case Member::PostSetAttrCallObject:
        {
            if( !PyCallable_Check( context ) )
            {
                cppy::type_error( context, "callable" );
                return false;
            }
            break;
        }
        case Member::PostSetAttrAtomMethod:
        case Member::PostSetAttrMemberMethod:
        {
            if( !Py23Str_Check( context ) )
            {
                cppy::type_error( context, "str" );
                return false;
            }
            break;
        }
        default:
        {
            break;
        }
    }
    return true;
}


PyObject* d_noop_h( Member* member, PyObject* atom, PyObject* name )
{
    return cppy::incref( Py_None );
}


PyObject* d_value_h( Member* member, PyObject* atom, PyObject* name )
{
    return cppy::incref( member->m_default_context );
}


PyObject* d_list_h( Member* member, PyObject* atom, PyObject* name )
{
    if( member->m_default_context == Py_None )
    {
        return PyList_New( 0 );
    }
    Py_ssize_t size = PyList_GET_SIZE( member->m_default_context );
    return PyList_GetSlice( member->m_default_context, 0, size );
}


PyObject* d_dict_h( Member* member, PyObject* atom, PyObject* name )
{
    if( member->m_default_context == Py_None )
    {
        return PyDict_New();
    }
    return PyDict_Copy( member->m_default_context );
}


PyObject* d_factory_h( Member* member, PyObject* atom, PyObject* name )
{
    cppy::ptr args( PyTuple_New( 0 ) );
    if( !args )
    {
        return 0;
    }
    return PyObject_Call( member->m_default_context, args.get(), 0 );
}


PyObject* d_call_object_h( Member* member, PyObject* atom, PyObject* name )
{
    cppy::ptr args( PyTuple_Pack( 3, member, atom, name ) );
    if( !args )
    {
        return 0;
    }
    return PyObject_Call( member->m_default_context, args.get(), 0 );
}


PyObject* d_atom_method_h( Member* member, PyObject* atom, PyObject* name )
{
    cppy::ptr method( PyObject_GetAttr( atom, member->m_default_context ) );
    if( !method )
    {
        return 0;
    }
    cppy::ptr args( PyTuple_New( 0 ) );
    if( !args )
    {
        return 0;
    }
    return method.call( args );
}


PyObject* d_member_method_h( Member* member, PyObject* atom, PyObject* name )
{
    cppy::ptr method( PyObject_GetAttr(
        reinterpret_cast<PyObject*>( member ), member->m_default_context ) );
    if( !method )
    {
        return 0;
    }
    cppy::ptr args( PyTuple_Pack( 2, atom, name ) );
    if( !args )
    {
        return 0;
    }
    return method.call( args );
}


PyObject* validation_error(
    Member* member, PyObject* atom, PyObject* name, PyObject* value )
{
    if( PyErr_Occurred() )
    {
        if( PyErr_ExceptionMatches( ValidationError ) )
        {
            PyErr_Clear();
        }
        else
        {
            return 0;
        }
    }
    PyObject* pymember = reinterpret_cast<PyObject*>( member );
    cppy::ptr method( PyObject_GetAttrString( pymember, "validation_error" ) );
    if( !method )
    {
        return 0;
    }
    cppy::ptr args( PyTuple_Pack( 3, atom, name, value ) );
    if( !args )
    {
        return 0;
    }
    cppy::ptr result( method.call( args ) );
    if( !result )
    {
        return 0;
    }
    cppy::system_error( "member failed raise validation error" );
    return 0;
}


PyObject* v_noop_h(
    Member* member, PyObject* atom, PyObject* name, PyObject* value )
{
    return cppy::incref( value );
}


PyObject* v_bool_h(
    Member* member, PyObject* atom, PyObject* name, PyObject* value )
{
    if( PyBool_Check( value ) )
    {
        return cppy::incref( value );
    }
    if( member->m_validate_context == Py_False ) // not strict
    {
        if( Py23Int_Check( value ) || PyFloat_Check( value ) )
        {
            long ok = PyObject_IsTrue( value );
            if( ok < 0 )
            {
                return 0;
            }
            return PyBool_FromLong( ok );
        }
    }
    return validation_error( member, atom, name, value );
}


PyObject* v_int_h(
    Member* member, PyObject* atom, PyObject* name, PyObject* value )
{
    if( Py23Int_Check( value ) )
    {
        return cppy::incref( value );
    }
    if( member->m_validate_context == Py_False ) // not strict
    {
        if( PyFloat_Check( value ) )
        {
            return Py23Number_Int( value );
        }
    }
    return validation_error( member, atom, name, value );
}


PyObject* v_float_h(
    Member* member, PyObject* atom, PyObject* name, PyObject* value )
{
    if( PyFloat_Check( value ) )
    {
        return cppy::incref( value );
    }
    if( member->m_validate_context == Py_False ) // not strict
    {
        if( Py23Int_Check( value ) )
        {
            return PyNumber_Float( value );
        }
    }
    return validation_error( member, atom, name, value );
}


PyObject* v_bytes_h(
    Member* member, PyObject* atom, PyObject* name, PyObject* value )
{
    if( Py23Bytes_Check( value ) )
    {
        return cppy::incref( value );
    }
    if( member->m_validate_context == Py_False ) // not strict
    {
        if( PyUnicode_Check( value ) )
        {
            return PyUnicode_AsUTF8String( value );
        }
    }
    return validation_error( member, atom, name, value );
}


PyObject* v_str_h(
    Member* member, PyObject* atom, PyObject* name, PyObject* value )
{
    if( Py23Str_Check( value ) )
    {
        return cppy::incref( value );
    }
    if( member->m_validate_context == Py_False ) // not strict
    {
#ifdef IS_PY3K
        if( PyBytes_Check( value ) )
        {
            return PyUnicode_FromString( PyBytes_AS_STRING( value ) );
        }
#else
        if( PyUnicode_Check( value ) )
        {
            return PyUnicode_AsUTF8String( value );
        }
#endif
    }
    return validation_error( member, atom, name, value );
}


PyObject* v_unicode_h(
    Member* member, PyObject* atom, PyObject* name, PyObject* value )
{
    if( PyUnicode_Check( value ) )
    {
        return cppy::incref( value );
    }
    if( member->m_validate_context == Py_False ) // not strict
    {
        if( Py23Bytes_Check( value ) )
        {
            return PyUnicode_FromString( Py23Bytes_AS_STRING( value ) );
        }
    }
    return validation_error( member, atom, name, value );
}


PyObject* v_tuple_h(
    Member* member, PyObject* atom, PyObject* name, PyObject* value )
{
    if( !PyTuple_Check( value ) )
    {
        return validation_error( member, atom, name, value );
    }
    if( member->m_validate_context == Py_None )
    {
        return cppy::incref( value );
    }
    Py_ssize_t size = PyTuple_GET_SIZE( value );
    cppy::ptr result( PyTuple_New( size ) );
    if( !result )
    {
        return 0;
    }
    Member* inner = reinterpret_cast<Member*>( member->m_validate_context );
    for( Py_ssize_t i = 0; i < size; ++i )
    {
        PyObject* item = PyTuple_GET_ITEM( value, i );
        PyObject* valid_item = inner->validateValue( atom, name, item );
        if( !valid_item )
        {
            return validation_error( member, atom, name, value );
        }
        PyTuple_SET_ITEM( result.get(), i, valid_item );
    }
    return result.release();
}


PyObject* v_list_h(
    Member* member, PyObject* atom, PyObject* name, PyObject* value )
{
    if( !PyList_Check( value ) )
    {
        return validation_error( member, atom, name, value );
    }
    Py_ssize_t size = PyList_GET_SIZE( value );
    if( member->m_validate_context == Py_None )
    {
        return PyList_GetSlice( value, 0, size );
    }
    cppy::ptr result( PyList_New( size ) );
    if( !result )
    {
        return 0;
    }
    Member* inner = reinterpret_cast<Member*>( member->m_validate_context );
    for( Py_ssize_t i = 0; i < size; ++i )
    {
        PyObject* item = PyList_GET_ITEM( value, i );
        PyObject* valid_item = inner->validateValue( atom, name, item );
        if( !valid_item )
        {
            return validation_error( member, atom, name, value );
        }
        PyList_SET_ITEM( result.get(), i, valid_item );
    }
    return result.release();
}


PyObject* v_dict_h(
    Member* member, PyObject* atom, PyObject* name, PyObject* value )
{
    if( !PyDict_Check( value ) )
    {
        return validation_error( member, atom, name, value );
    }
    return PyDict_Copy( value );
    // XXX handle dict types
}


PyObject* v_typed_h(
    Member* member, PyObject* atom, PyObject* name, PyObject* value )
{
    if( value == Py_None )
    {
        return cppy::incref( value );
    }
    PyObject* ctxt = member->m_validate_context;
    if( PyObject_TypeCheck( value, reinterpret_cast<PyTypeObject*>( ctxt ) ) )
    {
        return cppy::incref( value );
    }
    return validation_error( member, atom, name, value );
}


PyObject* v_instance_h(
    Member* member, PyObject* atom, PyObject* name, PyObject* value )
{
    if( value == Py_None )
    {
        return cppy::incref( value );
    }
    int r = PyObject_IsInstance( value, member->m_validate_context );
    if( r == 1 )
    {
        return cppy::incref( value );
    }
    if( r == -1 )
    {
        return 0;
    }
    return validation_error( member, atom, name, value );
}


PyObject* v_subclass_h(
    Member* member, PyObject* atom, PyObject* name, PyObject* value )
{
    if( value == Py_None )
    {
        return cppy::incref( value );
    }
    if( !PyType_Check( value ) )
    {
        return validation_error( member, atom, name, value );
    }
    int r = PyObject_IsSubclass( value, member->m_validate_context );
    if( r == 1 )
    {
        return cppy::incref( value );
    }
    if( r == -1 )
    {
        return 0;
    }
    return validation_error( member, atom, name, value );
}


PyObject* v_enum_h(
    Member* member, PyObject* atom, PyObject* name, PyObject* value )
{
    int r = PySequence_Contains( member->m_validate_context, value );
    if( r == 1 )
    {
        return cppy::incref( value );
    }
    if( r == -1 )
    {
        return 0;
    }
    return validation_error( member, atom, name, value );
}


PyObject* v_callable_h(
    Member* member, PyObject* atom, PyObject* name, PyObject* value )
{
    if( value == Py_None || PyCallable_Check( value ) )
    {
        return cppy::incref( value );
    }
    return validation_error( member, atom, name, value );
}


PyObject* v_range_h(
    Member* member, PyObject* atom, PyObject* name, PyObject* value )
{
    PyObject* minv = PyTuple_GET_ITEM( member->m_validate_context, 0 );
    PyObject* maxv = PyTuple_GET_ITEM( member->m_validate_context, 1 );
    PyObject* kind = PyTuple_GET_ITEM( member->m_validate_context, 2 );
    int r = PyObject_IsInstance( value, kind );
    if( r == 0 )
    {
        return validation_error( member, atom, name, value );
    }
    if( r == -1 )
    {
        return 0;
    }
    if( minv != Py_None )
    {
        r = PyObject_RichCompareBool( value, minv, Py_LT );
        if( r == 1 )
        {
            return validation_error( member, atom, name, value );
        }
        if( r == -1 )
        {
            return 0;
        }
    }
    if( maxv != Py_None )
    {
        r = PyObject_RichCompareBool( value, maxv, Py_GT );
        if( r == 1 )
        {
            return validation_error( member, atom, name, value );
        }
        if( r == -1 )
        {
            return 0;
        }
    }
    return cppy::incref( value );
}


PyObject* v_coerced_h(
    Member* member, PyObject* atom, PyObject* name, PyObject* value )
{
    PyObject* kind = PyTuple_GET_ITEM( member->m_validate_context, 0 );
    int r = PyObject_IsInstance( value, kind );
    if( r == 1 )
    {
        return cppy::incref( value );
    }
    if( r == -1 )
    {
        return 0;
    }
    cppy::ptr args( PyTuple_Pack( 1, value ) );
    if( !args )
    {
        return 0;
    }
    PyObject* coercer = PyTuple_GET_ITEM( member->m_validate_context, 1 );
    cppy::ptr result( PyObject_Call( coercer, args.get(), 0 ) );
    if( !result )
    {
        if( PyErr_ExceptionMatches( PyExc_TypeError ) ||
            PyErr_ExceptionMatches( PyExc_ValueError ) )
        {
            PyErr_Clear();
            return validation_error( member, atom, name, value );
        }
        return 0;
    }
    r = PyObject_IsInstance( result.get(), kind );
    if( r == 1 )
    {
        return result.release();
    }
    if( r == -1 )
    {
        return 0;
    }
    return validation_error( member, atom, name, value );
}


PyObject* v_call_object_h(
    Member* member, PyObject* atom, PyObject* name, PyObject* value )
{
    cppy::ptr args( PyTuple_Pack( 4, member, atom, name, value ) );
    if( !args )
    {
        return 0;
    }
    return PyObject_Call( member->m_validate_context, args.get(), 0 );
}


PyObject* v_atom_method_h(
    Member* member, PyObject* atom, PyObject* name, PyObject* value )
{
    cppy::ptr method( PyObject_GetAttr( atom, member->m_validate_context ) );
    if( !method )
    {
        return 0;
    }
    cppy::ptr args( PyTuple_Pack( 1, value ) );
    if( !args )
    {
        return 0;
    }
    return method.call( args );
}


PyObject* v_member_method_h(
    Member* member, PyObject* atom, PyObject* name, PyObject* value )
{
    cppy::ptr method( PyObject_GetAttr(
        reinterpret_cast<PyObject*>( member ), member->m_validate_context ) );
    if( !method )
    {
        return 0;
    }
    cppy::ptr args( PyTuple_Pack( 3, atom, name, value ) );
    if( !args )
    {
        return 0;
    }
    return method.call( args );
}


PyObject* pv_noop_h(
    Member* member, PyObject* atom, PyObject* name, PyObject* value )
{
    return cppy::incref( value );
}


PyObject* pv_call_object_h(
    Member* member, PyObject* atom, PyObject* name, PyObject* value )
{
    cppy::ptr args( PyTuple_Pack( 4, member, atom, name, value ) );
    if( !args )
    {
        return 0;
    }
    return PyObject_Call( member->m_post_validate_context, args.get(), 0 );
}


PyObject* pv_atom_method_h(
    Member* member, PyObject* atom, PyObject* name, PyObject* value )
{
    cppy::ptr method( PyObject_GetAttr(
        atom, member->m_post_validate_context ) );
    if( !method )
    {
        return 0;
    }
    cppy::ptr args( PyTuple_Pack( 1, value ) );
    if( !args )
    {
        return 0;
    }
    return method.call( args );
}


PyObject* pv_member_method_h(
    Member* member, PyObject* atom, PyObject* name, PyObject* value )
{
    cppy::ptr method( PyObject_GetAttr(
        reinterpret_cast<PyObject*>( member ),
        member->m_post_validate_context ) );
    if( !method )
    {
        return 0;
    }
    cppy::ptr args( PyTuple_Pack( 3, atom, name, value ) );
    if( !args )
    {
        return 0;
    }
    return method.call( args );
}


int ps_noop_h(
    Member* member, PyObject* atom, PyObject* name, PyObject* value )
{
    return 0;
}


int ps_call_object_h(
    Member* member, PyObject* atom, PyObject* name, PyObject* value )
{
    cppy::ptr args( PyTuple_Pack( 4, member, atom, name, value ) );
    if( !args )
    {
        return -1;
    }
    cppy::ptr res( PyObject_Call(
        member->m_post_setattr_context, args.get(), 0 ) );
    if( !res )
    {
        return -1;
    }
    return 0;
}


int ps_atom_method_h(
    Member* member, PyObject* atom, PyObject* name, PyObject* value )
{
    cppy::ptr method( PyObject_GetAttr(
        atom, member->m_post_setattr_context ) );
    if( !method )
    {
        return -1;
    }
    cppy::ptr args( PyTuple_Pack( 1, value ) );
    if( !args )
    {
        return -1;
    }
    cppy::ptr res( method.call( args ) );
    if( !res )
    {
        return -1;
    }
    return 0;
}


int ps_member_method_h(
    Member* member, PyObject* atom, PyObject* name, PyObject* value )
{
    cppy::ptr method( PyObject_GetAttr(
        reinterpret_cast<PyObject*>( member ),
        member->m_post_setattr_context ) );
    if( !method )
    {
        return -1;
    }
    cppy::ptr args( PyTuple_Pack( 3, atom, name, value ) );
    if( !args )
    {
        return -1;
    }
    cppy::ptr res( method.call( args ) );
    if( !res )
    {
        return -1;
    }
    return 0;
}


typedef PyObject* ( *DefaultHandler )(
    Member* member, PyObject* atom, PyObject* name );


typedef PyObject* ( *ValidateHandler )(
    Member* member, PyObject* atom, PyObject* name, PyObject* value );


typedef PyObject* ( *PostValidateHandler )(
    Member* member, PyObject* atom, PyObject* name, PyObject* value );


typedef int ( *PostSetAttrHandler )(
    Member* member, PyObject* atom, PyObject* name, PyObject* value );


DefaultHandler d_handlers[] = {
    d_noop_h,
    d_value_h,
    d_list_h,
    d_dict_h,
    d_factory_h,
    d_call_object_h,
    d_atom_method_h,
    d_member_method_h
};


ValidateHandler v_handlers[] = {
    v_noop_h,
    v_bool_h,
    v_int_h,
    v_float_h,
    v_bytes_h,
    v_str_h,
    v_unicode_h,
    v_tuple_h,
    v_list_h,
    v_dict_h,
    v_typed_h,
    v_instance_h,
    v_subclass_h,
    v_enum_h,
    v_callable_h,
    v_range_h,
    v_coerced_h,
    v_call_object_h,
    v_atom_method_h,
    v_member_method_h
};


PostValidateHandler pv_handlers[] = {
    pv_noop_h,
    pv_call_object_h,
    pv_atom_method_h,
    pv_member_method_h
};


PostSetAttrHandler ps_handlers[] = {
    ps_noop_h,
    ps_call_object_h,
    ps_atom_method_h,
    ps_member_method_h
};


void Member_clear( Member* self )
{
    Py_CLEAR( self->m_metadata );
    Py_CLEAR( self->m_default_context );
    Py_CLEAR( self->m_validate_context );
    Py_CLEAR( self->m_post_validate_context );
    Py_CLEAR( self->m_post_setattr_context );
}


int Member_traverse( Member* self, visitproc visit, void* arg )
{
    Py_VISIT( self->m_metadata );
    Py_VISIT( self->m_default_context );
    Py_VISIT( self->m_validate_context );
    Py_VISIT( self->m_post_validate_context );
    Py_VISIT( self->m_post_setattr_context );
    return 0;
}


void Member_dealloc( Member* self )
{
    PyObject_GC_UnTrack( self );
    Member_clear( self );
    self->ob_type->tp_free( reinterpret_cast<PyObject*>( self ) );
}


PyObject* Member_get_metadata( Member* self, void* context )
{
    PyObject* ob = self->m_metadata;
    return cppy::incref( ob ? ob : Py_None );
}


int Member_set_metadata( Member* self, PyObject* value, void* context )
{
    if( !value || value == Py_None )
    {
        cppy::clear( &self->m_metadata );
        return 0;
    }
    if( !PyDict_Check( value ) )
    {
        cppy::type_error( value, "dict or None" );
        return -1;
    }
    cppy::replace( &self->m_metadata, value );
    return 0;
}


template <typename MODE>
struct ModeTraits;


template <>
struct ModeTraits<Member::DefaultMode>
{
    static const Member::DefaultMode Last = Member::DefaultLast;
};


template <>
struct ModeTraits<Member::ValidateMode>
{
    static const Member::ValidateMode Last = Member::ValidateLast;
};


template <>
struct ModeTraits<Member::PostValidateMode>
{
    static const Member::PostValidateMode Last = Member::PostValidateLast;
};


template <>
struct ModeTraits<Member::PostSetAttrMode>
{
    static const Member::PostSetAttrMode Last = Member::PostSetAttrLast;
};


template <typename MODE>
struct PackMode
{
    PyObject* operator()( uint8_t mode, PyObject* context )
    {
        PyObject* pymode = Py23Int_FromLong( static_cast<long>( mode ) );
        if( !pymode )
        {
            return 0;
        }
        return PyTuple_Pack( 2, pymode, context ? context : Py_None );
    }
};


template <typename MODE>
struct ParseMode
{
    PyObject* operator()( PyObject* args, uint8_t* mode, PyObject** context )
    {
        long val;
        PyObject* ctxt;
        if( !PyArg_ParseTuple( args, "lO", &val, &ctxt ) )
        {
            return 0;
        }
        if( val < 0 || val >= ModeTraits<MODE>::Last )
        {
            return cppy::value_error( "mode out of range" );
        }
        if( !check_context( static_cast<MODE>( val ), ctxt ) )
        {
            return 0;
        }
        *mode = static_cast<uint8_t>( val );
        cppy::replace( context, ctxt );
        return cppy::incref( Py_None );
    }
};


PyObject* Member_default_mode( Member* self, PyObject* args )
{
    return PackMode<Member::DefaultMode>()(
        self->m_default_mode, self->m_default_context );
}


PyObject* Member_set_default_mode( Member* self, PyObject* args )
{
    return ParseMode<Member::DefaultMode>()(
        args, &self->m_default_mode, &self->m_default_context );
}


PyObject* Member_validate_mode( Member* self, PyObject* args )
{
    return PackMode<Member::ValidateMode>()(
        self->m_validate_mode, self->m_validate_context );
}


PyObject* Member_set_validate_mode( Member* self, PyObject* args )
{
    return ParseMode<Member::ValidateMode>()(
        args, &self->m_validate_mode, &self->m_validate_context );
}


PyObject* Member_post_validate_mode( Member* self, PyObject* args )
{
    return PackMode<Member::PostValidateMode>()(
        self->m_post_validate_mode, self->m_post_validate_context );
}


PyObject* Member_set_post_validate_mode( Member* self, PyObject* args )
{
    return ParseMode<Member::PostValidateMode>()(
        args, &self->m_post_validate_mode, &self->m_post_validate_context );
}


PyObject* Member_post_setattr_mode( Member* self, PyObject* args )
{
    return PackMode<Member::PostSetAttrMode>()(
        self->m_post_setattr_mode, self->m_post_setattr_context );
}


PyObject* Member_set_post_setattr_mode( Member* self, PyObject* args )
{
    return ParseMode<Member::PostSetAttrMode>()(
        args, &self->m_post_setattr_mode, &self->m_post_setattr_context );
}


PyObject* Member_clone( Member* self, PyObject* args )
{
    cppy::ptr pyclone( PyType_GenericNew( self->ob_type, 0, 0 ) );
    if( !pyclone )
    {
        return 0;
    }
    Member* clone = reinterpret_cast<Member*>( pyclone.get() );
    if( self->m_metadata )
    {
        clone->m_metadata = PyDict_Copy( self->m_metadata );
        if( !clone->m_metadata )
        {
            return 0;
        }
    }
    clone->m_default_context = cppy::xincref( self->m_default_context );
    clone->m_validate_context = cppy::xincref( self->m_validate_context );
    clone->m_post_validate_context = cppy::xincref( self->m_post_validate_context );
    clone->m_post_setattr_context = cppy::xincref( self->m_post_setattr_context );
    clone->m_flags = self->m_flags;
    clone->m_default_mode = self->m_default_mode;
    clone->m_validate_mode = self->m_validate_mode;
    clone->m_post_validate_mode = self->m_post_validate_mode;
    clone->m_post_setattr_mode = self->m_post_setattr_mode;
    return pyclone.release();
}


PyGetSetDef Member_getset[] = {
    { "metadata",
      ( getter )Member_get_metadata,
      ( setter )Member_set_metadata,
      "metadata for the member (if defined)" },
    { 0 } // sentinel
};


PyMethodDef Member_methods[] = {
    { "default_mode",
      ( PyCFunction )Member_default_mode,
      METH_NOARGS,
      "Get the default mode for the member." },
    { "set_default_mode",
      ( PyCFunction )Member_set_default_mode,
      METH_VARARGS,
      "Set the default mode for the member." },
    { "validate_mode",
      ( PyCFunction )Member_validate_mode,
      METH_NOARGS,
      "Get the validate mode for the member." },
    { "set_validate_mode",
      ( PyCFunction )Member_set_validate_mode,
      METH_VARARGS,
      "Set the validate mode for the member." },
    { "post_validate_mode",
      ( PyCFunction )Member_post_validate_mode,
      METH_NOARGS,
      "Get the post validate mode for the member." },
    { "set_post_validate_mode",
      ( PyCFunction )Member_set_post_validate_mode,
      METH_VARARGS,
      "Set the post validate mode for the member." },
    { "post_setattr_mode",
      ( PyCFunction )Member_post_setattr_mode,
      METH_NOARGS,
      "Get the post setattr mode for the member." },
    { "set_post_setattr_mode",
      ( PyCFunction )Member_set_post_setattr_mode,
      METH_VARARGS,
      "Set the post setattr mode for the member." },
    { "clone",
      ( PyCFunction )Member_clone,
      METH_NOARGS,
      "Create a clone of the member." },
    { 0 } // sentinel
};


template <typename MODE>
bool add_member_mode( const char* name, MODE mode )
{
    cppy::ptr value( Py23Int_FromLong( mode ) );
    if( !value )
    {
        return false;
    }
    PyObject* type_dict = Member::TypeObject.tp_dict;
    return PyDict_SetItemString( type_dict, name, value.get() ) == 0;
}

} // namespace


PyTypeObject Member::TypeObject = {
    PyObject_HEAD_INIT( &PyType_Type )
    0,                                     /* ob_size */
    "atom.catom.CMember",                  /* tp_name */
    sizeof( Member ),                      /* tp_basicsize */
    0,                                     /* tp_itemsize */
    ( destructor )Member_dealloc,          /* tp_dealloc */
    ( printfunc )0,                        /* tp_print */
    ( getattrfunc )0,                      /* tp_getattr */
    ( setattrfunc )0,                      /* tp_setattr */
    ( cmpfunc )0,                          /* tp_compare */
    ( reprfunc )0,                         /* tp_repr */
    ( PyNumberMethods* )0,                 /* tp_as_number */
    ( PySequenceMethods* )0,               /* tp_as_sequence */
    ( PyMappingMethods* )0,                /* tp_as_mapping */
    ( hashfunc )0,                         /* tp_hash */
    ( ternaryfunc )0,                      /* tp_call */
    ( reprfunc )0,                         /* tp_str */
    ( getattrofunc )0,                     /* tp_getattro */
    ( setattrofunc )0,                     /* tp_setattro */
    ( PyBufferProcs* )0,                   /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT
    | Py_TPFLAGS_BASETYPE
    | Py_TPFLAGS_HAVE_GC
    | Py_TPFLAGS_HAVE_VERSION_TAG,         /* tp_flags */
    0,                                     /* Documentation string */
    ( traverseproc )Member_traverse,       /* tp_traverse */
    ( inquiry )Member_clear,               /* tp_clear */
    ( richcmpfunc )0,                      /* tp_richcompare */
    0,                                     /* tp_weaklistoffset */
    ( getiterfunc )0,                      /* tp_iter */
    ( iternextfunc )0,                     /* tp_iternext */
    ( struct PyMethodDef* )Member_methods, /* tp_methods */
    ( struct PyMemberDef* )0,              /* tp_members */
    Member_getset,                         /* tp_getset */
    0,                                     /* tp_base */
    0,                                     /* tp_dict */
    ( descrgetfunc )0,                     /* tp_descr_get */
    ( descrsetfunc )0,                     /* tp_descr_set */
    0,                                     /* tp_dictoffset */
    ( initproc )0,                         /* tp_init */
    ( allocfunc )PyType_GenericAlloc,      /* tp_alloc */
    ( newfunc )PyType_GenericNew,          /* tp_new */
    ( freefunc )PyObject_GC_Del,           /* tp_free */
    ( inquiry )0,                          /* tp_is_gc */
    0,                                     /* tp_bases */
    0,                                     /* tp_mro */
    0,                                     /* tp_cache */
    0,                                     /* tp_subclasses */
    0,                                     /* tp_weaklist */
    ( destructor )0                        /* tp_del */
};


bool Member::Ready()
{
    if( PyType_Ready( &TypeObject ) != 0 )
    {
        return false;
    }
    ValidationError = PyErr_NewException( "catom.ValidationError", 0, 0 );
    if( !ValidationError )
    {
        return false;
    }

#define STR_HELPER( x ) #x
#define STR( x ) STR_HELPER( x )
#define ADD_HELPER( a, b ) if( !add_member_mode( a, b ) ) return false;
#define ADD_MODE( m ) ADD_HELPER( STR( m ), m )

    ADD_MODE( NoDefault )
    ADD_MODE( DefaultValue )
    ADD_MODE( DefaultList )
    ADD_MODE( DefaultDict )
    ADD_MODE( DefaultFactory )
    ADD_MODE( DefaultCallObject )
    ADD_MODE( DefaultAtomMethod )
    ADD_MODE( DefaultMemberMethod )
    ADD_MODE( NoValidate )
    ADD_MODE( ValidateBool )
    ADD_MODE( ValidateInt )
    ADD_MODE( ValidateFloat )
    ADD_MODE( ValidateBytes )
    ADD_MODE( ValidateStr )
    ADD_MODE( ValidateUnicode )
    ADD_MODE( ValidateTuple )
    ADD_MODE( ValidateList )
    ADD_MODE( ValidateDict )
    ADD_MODE( ValidateTyped )
    ADD_MODE( ValidateInstance )
    ADD_MODE( ValidateSubclass )
    ADD_MODE( ValidateEnum )
    ADD_MODE( ValidateCallable )
    ADD_MODE( ValidateRange )
    ADD_MODE( ValidateCoerced )
    ADD_MODE( ValidateCallObject )
    ADD_MODE( ValidateAtomMethod )
    ADD_MODE( ValidateMemberMethod )
    ADD_MODE( NoPostValidate )
    ADD_MODE( PostValidateCallObject )
    ADD_MODE( PostValidateAtomMethod )
    ADD_MODE( PostValidateMemberMethod )
    ADD_MODE( NoPostSetAttr )
    ADD_MODE( PostSetAttrCallObject )
    ADD_MODE( PostSetAttrAtomMethod )
    ADD_MODE( PostSetAttrMemberMethod )

#undef ADD_MODE
#undef ADD_HELPER
#undef STR
#undef STR_HELPER

    return true;
}


PyObject* Member::defaultValue( PyObject* atom, PyObject* name )
{
    cppy::ptr result( Py_None, true );
    if( m_default_mode )
    {
        result = d_handlers[ m_default_mode ]( this, atom, name );
        if( !result )
        {
            return 0;
        }
    }
    return validateValue( atom, name, result.get() );
}


PyObject* Member::validateValue(
    PyObject* atom, PyObject* name, PyObject* value )
{
    cppy::ptr result( value, true );
    if( m_validate_mode )
    {
        result = v_handlers[ m_validate_mode ](
            this, atom, name, result.get() );
        if( !result )
        {
            return 0;
        }
    }
    if( m_post_validate_mode )
    {
        result = pv_handlers[ m_post_validate_mode ](
            this, atom, name, result.get() );
        if( !result )
        {
            return 0;
        }
    }
    return result.release();
}


int Member::postSetAttrValue( PyObject* atom, PyObject* name, PyObject* value )
{
    if( m_post_setattr_mode )
    {
        return ps_handlers[ m_post_setattr_mode ]( this, atom, name, value );
    }
    return 0;
}

} // namespace atom
