/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include <cppy/cppy.h>
#include <utils/py23compat.h>
#include "member.h"


namespace atom
{

namespace
{

bool check_context( Member::DefaultMode mode, PyObject* context )
{
    switch( mode )
    {
    case Member::DefaultList:
        if( context != Py_None && !PyList_Check( context ) )
        {
            cppy::type_error( context, "list" );
            return false;
        }
        break;
    case Member::DefaultDict:
        if( context != Py_None && !PyDict_Check( context ) )
        {
            cppy::type_error( context, "dict" );
            return false;
        }
        break;
    case Member::DefaultFactory:
        if( !PyCallable_Check( context ) )
        {
            cppy::type_error( context, "callable" ):
            return false;
        }
        break;
    case Member::DefaultAtomMethod:
    case Member::DefaultMemberMethod:
        if( !Py23Str_Check( context ) )
        {
            cppy::type_error( context, "str" );
            return false;
        }
        break;
    default:
        break;
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
        if( context != Py_True && context != Py_False )
        {
            cppy::type_error( context, "bool" );
            return false;
        }
        break;
    case Member::ValidateTuple:
    case Member::ValidateList:
        if( context != Py_None && !Member::TypeCheck( context ) )
        {
            cppy::type_error( context, "Member or None" );
            return false;
        }
        break;
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
        if( !PyType_Check( context ) )
        {
            cppy::type_error( context, "type" );
            return false;
        }
        break;
    // XXX validate a valid isinstance context for Instance?
    // XXX validate a valid issubclass context for Subclass?
    case Member::ValidateEnum:
        if( !PySequence_Check( context ) )
        {
            cppy::type_error( context, "sequence" );
            return false;
        }
        break;
    case Member::ValidateRange:
    {
        if( !PyTuple_Check( context ) || PyTuple_GET_SIZE( context ) != 3 )
        {
            cppy::type_error( context, "3-tuple of (low, high, member or None)" );
            return false;
        }
        PyObject* kind = PyTuple_GET_ITEM( context, 2 );
        if( kind != Py_None && !Member::TypeCheck( kind ) )
        {
            cppy::type_error( context, "3-tuple of (low, high, member or None)" );
            return false;
        }
        break;
    }
    case Member::ValidateCoerced:
    {
        if( !PyTuple_Check( context ) || PyTuple_GET_SIZE( context ) != 2 )
        {
            cppy::type_error( context, "2-tuple of (member, callable)" );
            return false;
        }
        PyObject* kind = PyTuple_GET_ITEM( context, 0 );
        PyObject* coercer = PyTuple_GET_ITEM( context, 1 );
        if( !Member::TypeCheck( kind ) || !PyCallable_Check( coercer ) )
        {
            cppy::type_error( context, "2-tuple of (member, callable)" );
            return false;
        }
        break;
    }
    case Member::ValidateAtomMethod:
    case Member::ValidateMemberMethod:
        if( !Py23Str_Check( context ) )
        {
            cppy::type_error( context, "str" );
            return false;
        }
        break;
    default:
        break;
    }
    return true;
}


bool check_context( Member::PostValidateMode mode, PyObject* context )
{
    switch( mode )
    {
    case Member::PostValidateAtomMethod:
    case Member::PostValidateMemberMethod:
        if( !Py23Str_Check( context ) )
        {
            cppy::type_error( context, "str" );
            return false;
        }
        break;
    default:
        break;
    }
    return true;
}


bool check_context( Member::PostSetAttrMode mode, PyObject* context )
{
    switch( mode )
    {
    case Member::PostSetAttrAtomMethod:
    case Member::PostSetAttrMemberMethod:
        if( !Py23Str_Check( context ) )
        {
            cppy::type_error( context, "str" );
            return false;
        }
        break;
    default:
        break;
    }
    return true;
}


PyObject* d_noop_h( Member* member, Atom* atom, PyObject* name )
{
    return cppy::incref( Py_None );
}


PyObject* d_value_h( Member* member, Atom* atom, PyObject* name )
{
    return cppy::incref( member->m_default_context );
}


PyObject* d_list_h( Member* member, Atom* atom, PyObject* name )
{
    if( member->m_default_context == Py_None )
    {
        return PyList_New( 0 );
    }
    Py_ssize_t size = PyList_GET_SIZE( member->m_default_context );
    return PyList_GetSlice( member->m_default_context, 0, size );
}


PyObject* d_dict_h( Member* member, Atom* atom, PyObject* name )
{
    if( member->m_default_context == Py_None )
    {
        return PyDict_New();
    }
    return PyDict_Copy( member->m_default_context );
}


PyObject* d_factory_h( Member* member, Atom* atom, PyObject* name )
{
    cppy::ptr args( PyTuple_New( 0 ) )
    if( !args )
    {
        return 0;
    }
    return PyObject_Call( member->m_default_context, args.get(), 0 );
}


PyObject* d_atom_method_h( Member* member, Atom* atom, PyObject* name )
{
    cppy::ptr method( PyObject_GetAttr(
        reinterpret_cast<PyObject*>( atom ), member->m_default_context ) );
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


PyObject* d_member_method_h( Member* member, Atom* atom, PyObject* name )
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


PyObject* v_noop_h(
    Member* member, Atom* atom, PyObject* name, PyObject* value )
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
    if( member->m_validate_context == Py_True )
    {
        long ok = PyObject_IsTrue( value );
        if( ok < 0 )
        {
            return 0;
        }
        return PyBool_FromLong( ok );
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
    if( member->m_validate_context == Py_True )
    {
        return Py23Number_Int( value );
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
    if( member->m_validate_context == Py_True )
    {
        return PyNumber_Float( value );
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
    return validation_error( member, atom, name, value );
}


PyObject* v_str_h(
    Member* member, PyObject* atom, PyObject* name, PyObject* value )
{
    if( Py23Str_Check( value ) )
    {
        return cppy::incref( value );
    }
#ifndef IS_PY3K
    if( PyUnicode_Check( value ) )
    {
        return PyUnicode_AsUTF8String( value );
    }
#endif
    return validation_error( member, atom, name, value );
}


PyObject* v_unicode_h(
    Member* member, PyObject* atom, PyObject* name, PyObject* value )
{
    if( PyUnicode_Check( value ) )
    {
        return cppy::incref( value );
    }
#ifndef IS_PY3K
    if( PyString_Check( value ) )
    {
        return PyUnicode_FromString( PyString_AS_STRING( value ) );
    }
#endif
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
        PyObject* valid_item = inner->validateValue(
            atom, name, PyTuple_GET_ITEM( value, i ) );
        if( !item )
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
    return PyList_GetSlice( value, 0, PyList_GET_SIZE( value ) );
    // XXX handle list types
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
    if( PyObject_TypeCheck( value,
        reinterpret_cast<PyTypeObject*>( member->m_validate_context ) ) )
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


PyObject* range_handler(
    Member* member, PyObject* atom, PyObject* name, PyObject* value )
{
    PyObject* minv = PyTuple_GET_ITEM( member->m_validate_context, 0 );
    PyObject* maxv = PyTuple_GET_ITEM( member->m_validate_context, 1 );
    Member* inner = reinterpret_cast<Member*>(
        PyTuple_GET_ITEM( member->m_validate_context, 2 ) );
    cppy::ptr valid( inner->validateValue( atom, name, value ) );
    if( !valid )
    {
        return validation_error( member, atom, name, value );
    }
    if( minv != Py_None )
    {
        int r = PyObject_RichCompareBool( valid.get(), minv, Py_LT );
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
        int r = PyObject_RichCompareBool( valid.get(), maxv, Py_GT );
        if( r == 1 )
        {
            return validation_error( member, atom, name, value );
        }
        if( r == -1 )
        {
            return 0;
        }
    }
    return valid.release();
}


PyObject* coerced_handler(
    Member* member, PyObject* atom, PyObject* name, PyObject* value )
{
    PyObject* inner = reinterpret_cast<Member*>(
        PyTuple_GET_ITEM( self->m_validate_context, 0 ) );
    PyObject* coercer = PyTuple_GET_ITEM( self->m_validate_context, 1 );
    int r = PyObject_IsInstance( value, type );
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
    PyObject* coercer = PyTuple_GET_ITEM( self->m_context, 1 );
    cppy::ptr result( PyObject_Call( coercer, args.get(), 0 ) );
    if( !result )
    {
        return 0;
    }
    r = PyObject_IsInstance( result.get(), type );
    if( r == 1 )
    {
        return result.release();
    }
    if( r == -1 )
    {
        return 0;
    }
    RAISE_VALIDATION_ERROR;
}


PyObject* v_atom_method_h(
    Member* member, Atom* atom, PyObject* name, PyObject* value )
{
    cppy::ptr method( PyObject_GetAttr(
        reinterpret_cast<PyObject*>( atom ), member->m_validate_context ) );
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
    Member* member, Atom* atom, PyObject* name, PyObject* value )
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
    Member* member, Atom* atom, PyObject* name, PyObject* value )
{
    return cppy::incref( value );
}


PyObject* pv_atom_method_h(
    Member* member, Atom* atom, PyObject* name, PyObject* value )
{
    cppy::ptr method( PyObject_GetAttr(
        reinterpret_cast<PyObject*>( atom ),
        member->m_post_validate_context ) );
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
    Member* member, Atom* atom, PyObject* name, PyObject* value )
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
    Member* member, Atom* atom, PyObject* name, PyObject* value )
{
    return 0;
}


int ps_atom_method_h(
    Member* member, Atom* atom, PyObject* name, PyObject* value )
{
    cppy::ptr method( PyObject_GetAttr(
        reinterpret_cast<PyObject*>( atom ),
        member->m_post_setattr_context ) );
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
    Member* member, Atom* atom, PyObject* name, PyObject* value )
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


DefaultHandler default_handlers[] = {
    d_noop_h,
    d_value_h,
    d_list_h,
    d_dict_h,
    d_factory_h,
    d_atom_method_h,
    d_member_method_h
};


ValidateHandler validate_handlers[] = {
    v_noop_h,
    v_atom_method_h,
    v_member_method_h
};


PostValidateHandler post_validate_handlers = {
    pv_noop_h,
    pv_atom_method_h,
    pv_member_method_h
};


PostSetAttrHandler post_setattr_handlers = {
    ps_noop_h,
    ps_atom_method_h,
    ps_member_method_h
};


int Member_init( PyObject* self, PyObject* args, PyObject* kwargs )
{
    if( PyTuple_GET_SIZE( args ) > 0 )
    {
        cppy::type_error( "__init__() takes no positional arguments" );
        return -1;
    }
    if( kwargs )
    {
        PyObject* key;
        PyObject* value;
        Py_ssize_t pos = 0;
        while( PyDict_Next( kwargs, &pos, &key, &value ) )
        {
            if( PyObject_SetAttr( self, key, value ) < 0 )
            {
                return -1;
            }
        }
    }
    return 0;
}


void Member_clear( Member* self )
{
    Py_CLEAR( self->m_default );
    Py_CLEAR( self->m_validate );
    Py_CLEAR( self->m_post_validate );
    Py_CLEAR( self->m_post_setattr );
}


int Member_traverse( Member* self, visitproc visit, void* arg )
{
    Py_VISIT( self->m_default );
    Py_VISIT( self->m_validate );
    Py_VISIT( self->m_post_validate );
    Py_VISIT( self->m_post_setattr );
    return 0;
}


void Member_dealloc( Member* self )
{
    PyObject_GC_UnTrack( self );
    Member_clear( self );
    self->ob_type->tp_free( reinterpret_cast<PyObject*>( self ) );
}

} // namespace


PyTypeObject Member::TypeObject = {
    PyObject_HEAD_INIT( &PyType_Type )
    0,                                 /* ob_size */
    "atom.catom.CMember",              /* tp_name */
    sizeof( Member ),                  /* tp_basicsize */
    0,                                 /* tp_itemsize */
    ( destructor )Member_dealloc,      /* tp_dealloc */
    ( printfunc )0,                    /* tp_print */
    ( getattrfunc )0,                  /* tp_getattr */
    ( setattrfunc )0,                  /* tp_setattr */
    ( cmpfunc )0,                      /* tp_compare */
    ( reprfunc )0,                     /* tp_repr */
    ( PyNumberMethods* )0,             /* tp_as_number */
    ( PySequenceMethods* )0,           /* tp_as_sequence */
    ( PyMappingMethods* )0,            /* tp_as_mapping */
    ( hashfunc )0,                     /* tp_hash */
    ( ternaryfunc )0,                  /* tp_call */
    ( reprfunc )0,                     /* tp_str */
    ( getattrofunc )0,                 /* tp_getattro */
    ( setattrofunc )0,                 /* tp_setattro */
    ( PyBufferProcs* )0,               /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT
    | Py_TPFLAGS_BASETYPE
    | Py_TPFLAGS_HAVE_GC
    | Py_TPFLAGS_HAVE_VERSION_TAG,     /* tp_flags */
    0,                                 /* Documentation string */
    ( traverseproc )Member_traverse,   /* tp_traverse */
    ( inquiry )Member_clear,           /* tp_clear */
    ( richcmpfunc )0,                  /* tp_richcompare */
    0,                                 /* tp_weaklistoffset */
    ( getiterfunc )0,                  /* tp_iter */
    ( iternextfunc )0,                 /* tp_iternext */
    ( struct PyMethodDef* )0,          /* tp_methods */
    ( struct PyMemberDef* )0,          /* tp_members */
    0,                                 /* tp_getset */
    0,                                 /* tp_base */
    0,                                 /* tp_dict */
    ( descrgetfunc )0,                 /* tp_descr_get */
    ( descrsetfunc )0,                 /* tp_descr_set */
    0,                                 /* tp_dictoffset */
    ( initproc )Member_init,           /* tp_init */
    ( allocfunc )PyType_GenericAlloc,  /* tp_alloc */
    ( newfunc )PyType_GenericNew,      /* tp_new */
    ( freefunc )PyObject_GC_Del,       /* tp_free */
    ( inquiry )0,                      /* tp_is_gc */
    0,                                 /* tp_bases */
    0,                                 /* tp_mro */
    0,                                 /* tp_cache */
    0,                                 /* tp_subclasses */
    0,                                 /* tp_weaklist */
    ( destructor )0                    /* tp_del */
};


bool Member::Ready()
{
    return PyType_Ready( &TypeObject ) == 0;
}


PyObject* Member::getDefault( PyObject* atom, PyObject* name )
{
    cppy::ptr result( Py_None, true );
    if( m_default )
    {
        cppy::ptr args( PyTuple_Pack( 2, atom, name ) );
        if( !args )
        {
            return 0;
        }
        result = PyObject_Call( m_default, args.get(), 0 );
        if( !result )
        {
            return 0;
        }
    }
    return validate( atom, name, result.get() );
}


PyObject* Member::validate( PyObject* atom, PyObject* name, PyObject* value )
{
    cppy::ptr result( value, true );
    if( m_validate )
    {
        cppy::ptr args( PyTuple_Pack( 3, atom, name, result.get() ) );
        if( !args )
        {
            return 0;
        }
        result = PyObject_Call( m_validate, args.get(), 0 );
        if( !result )
        {
            return 0;
        }
    }
    if( m_post_validate )
    {
        cppy::ptr args( PyTuple_Pack( 3, atom, name, result.get() ) );
        if( !args )
        {
            return 0;
        }
        result = PyObject_Call( m_post_validate, args.get(), 0 );
        if( !result )
        {
            return 0;
        }
    }
    return result.release();
}


int Member::postSetAttr( PyObject* atom, PyObject* name, PyObject* value )
{
    if( m_post_setattr )
    {
        cppy::ptr args( PyTuple_Pack( 3, atom, name, value ) );
        if( !args )
        {
            return 0;
        }
        cppy::ptr result( PyObject_Call( m_post_setattr, args.get(), 0 ) );
        if( !result )
        {
            return -1;
        }
    }
    return 0;
}

} // namespace atom
