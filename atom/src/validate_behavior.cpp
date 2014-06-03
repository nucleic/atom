/*-----------------------------------------------------------------------------
| Copyright (c) 2013, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include <limits>
#include <Python.h>
#include "pythonhelpers.h"
#include "validate_behavior.h"
#include "member.h"
#include "atomlist.h"


using namespace PythonHelpers;


// true on success, false and exception on failure
bool Validate_CheckMode( Validate::Mode mode, PyObject* context )
{
    switch( mode )
    {
    case Validate::Tuple:
    case Validate::List:
    case Validate::ContainerList:
        if( context != Py_None && !Member_Check( context ) )
        {
            py_expected_type_fail( context, "Member or None" );
            return false;
        }
        break;
    case Validate::Dict:
    {
        if( !PyTuple_Check( context ) || PyTuple_GET_SIZE( context ) != 2 )
        {
            py_expected_type_fail( context, "2-tuple of Member or None" );
            return false;
        }
        PyObject* k = PyTuple_GET_ITEM( context, 0 );
        PyObject* v = PyTuple_GET_ITEM( context, 1 );
        if( ( k != Py_None && !Member_Check( k ) ) ||
            ( v != Py_None && !Member_Check( v ) ) )
        {
            py_expected_type_fail( context, "2-tuple of Member or None" );
            return false;
        }
        break;
    }
    // XXX validate a valid isinstance type?
    case Validate::Typed:
        if( !PyType_Check( context ) )
        {
            py_expected_type_fail( context, "type" );
            return false;
        }
        break;
    // XXX validate a valid subclass type?
    case Validate::Enum:
        if( !PySequence_Check( context ) )
        {
            py_expected_type_fail( context, "sequence" );
            return false;
        }
        break;
    case Validate::Range:
    {
        if( !PyTuple_Check( context ) || PyTuple_GET_SIZE( context ) != 2 )
        {
            py_expected_type_fail( context, "2-tuple of int or None" );
            return false;
        }
        PyObject* s = PyTuple_GET_ITEM( context, 0 );
        PyObject* e = PyTuple_GET_ITEM( context, 1 );
        if( ( s != Py_None && !PyInt_Check( s ) ) ||
            ( e != Py_None && !PyInt_Check( s ) ) )
        {
            py_expected_type_fail( context, "2-tuple of int or None" );
            return false;
        }
        break;
    }
    case Validate::FloatRange:
    {
        if( !PyTuple_Check( context ) || PyTuple_GET_SIZE( context ) != 2 )
        {
            py_expected_type_fail( context, "2-tuple of float or None" );
            return false;
        }
        PyObject* s = PyTuple_GET_ITEM( context, 0 );
        PyObject* e = PyTuple_GET_ITEM( context, 1 );
        if( ( s != Py_None && !PyFloat_Check( s ) ) ||
            ( e != Py_None && !PyFloat_Check( e ) ) )
        {
            py_expected_type_fail( context, "2-tuple of float or None" );
            return false;
        }
        break;
    }
    case Validate::Coerced:
    {
        if( !PyTuple_Check( context ) || PyTuple_GET_SIZE( context ) != 2 )
        {
            py_expected_type_fail( context, "2-tuple of (type, callable)" );
            return false;
        }
        PyObject* t = PyTuple_GET_ITEM( context, 0 );
        PyObject* c = PyTuple_GET_ITEM( context, 1 );
        // XXX validate type as valid for isinstance(..., t)
        if( !PyCallable_Check( c ) )
        {
            py_expected_type_fail( context, "2-tuple of (type, callable)" );
            return false;
        }
        break;
    }
    case Validate::ObjectMethod:
    case Validate::MemberMethod:
        if( !PyString_Check( context ) )
        {
            py_expected_type_fail( context, "str" );
            return false;
        }
        break;
    default:
        break;
    }
    return true;
}


static PyObject* validation_error( Member* member,
                                   CAtom* atom,
                                   PyStringObject* name,
                                   PyObject* value )
{
    PyErr_Clear();
    PyObjectPtr ignored( PyObject_CallMethod( ( PyObject* )member,
                                              "validation_error",
                                              "OOO",
                                              ( PyObject* )atom,
                                              ( PyObject* )name,
                                              value ) );
    if( !PyErr_Occurred() )
    {
        PyErr_SetString( PyExc_TypeError,
                         "member failed to raise validation error" )
    }
    return 0;
}


#define VALIDATION_ERROR return validation_error( member, atom, name, value )


static PyObject* no_op_handler( Member* member,
                                CAtom* atom,
                                PyStringObject* name,
                                PyObject* value )
{
    return newref( value );
}


static PyObject* bool_handler( Member* member,
                               CAtom* atom,
                               PyStringObject* name,
                               PyObject* value )
{
    if( PyBool_Check( value ) )
    {
        return newref( value );
    }
    VALIDATION_ERROR;
}


static PyObject* int_handler( Member* member,
                              CAtom* atom,
                              PyStringObject* name,
                              PyObject* value )
{
    if( PyInt_Check( value ) || PyLong_Check( value ) )
    {
        return newref( value );
    }
    VALIDATION_ERROR;
}


static PyObject* int_cast_handler( Member* member,
                                   CAtom* atom,
                                   PyStringObject* name,
                                   PyObject* value )
{
    if( PyInt_Check( value ) || PyLong_Check( value ) )
    {
        return newref( value );
    }
    if( PyFloat_Check( value ) )
    {
        PyObject* result = PyNumber_Int( value );
        if( result )
        {
            return result;
        }
    }
    VALIDATION_ERROR;
}


static PyObject* float_handler( Member* member,
                                CAtom* atom,
                                PyStringObject* name,
                                PyObject* value )
{
    if( PyFloat_Check( value ) )
    {
        return newref( value );
    }
    VALIDATION_ERROR;
}


static PyObject* float_cast_handler( Member* member,
                                     CAtom* atom,
                                     PyStringObject* name,
                                     PyObject* value )
{
    if( PyFloat_Check( value ) )
    {
        return newref( value );
    }
    if( PyInt_Check( value ) || PyLong_Check( value ) )
    {
        PyObject* result = PyNumber_Float( value );
        if( result )
        {
            return result;
        }
    }
    VALIDATION_ERROR;
}


static PyObject* bytes_handler( Member* member,
                                CAtom* atom,
                                PyStringObject* name,
                                PyObject* value )
{
    if( PyString_Check( value ) )
    {
        return newref( value );
    }
    VALIDATION_ERROR;
}


static PyObject* string_handler( Member* member,
                                 CAtom* atom,
                                 PyStringObject* name,
                                 PyObject* value )
{
    if( PyUnicode_Check( value ) )
    {
        return newref( value );
    }
    VALIDATION_ERROR;
}


static PyObject* tuple_handler( Member* member,
                                CAtom* atom,
                                PyStringObject* name,
                                PyObject* value )
{
    if( !PyTuple_Check( value ) )
    {
        VALIDATION_ERROR;
    }
    if( member->validate_context == PyNone )
    {
        return newref( value );
    }
    Py_ssize_t size = PyTuple_GET_SIZE( value );
    PyObjectPtr result( PyTuple_New( size ) );
    if( !result )
    {
        return 0; // a memory error is not a validation failure
    }
    Member* im = ( Member* )member->validate_context;
    for( Py_ssize_t i = 0; i < size; ++i )
    {
        PyObject* item = PyTuple_GET_ITEM( value, i );
        item = Member_FullValidate( im, atom, name, item );
        if( !item )
        {
            VALIDATION_ERROR;
        }
        PyTuple_SET_ITEM( result.get(), i, item );
    }
    return result.release();
}


template <typename ListFactory>
PyObject* common_list_handler( Member* member,
                               CAtom* atom,
                               PyStringObject* name,
                               PyObject* value )
{
    if( !PyList_Check( value ) )
    {
        VALIDATION_ERROR;
    }
    Member* im = 0;
    if( member->validate_context != Py_None )
    {
        im = ( Member* )member->validate_context;
    }
    Py_ssize_t size = PyList_GET_SIZE( value );
    PyObjectPtr result( ListFactory()( member, atom, im, size ) );
    if( !result )
    {
        return 0; // a memory error is not a validation failure
    }
    if( !im )
    {
        for( Py_ssize_t i = 0; i < size; ++i )
        {
            PyObject* item = PyList_GET_ITEM( value, i );
            PyList_SET_ITEM( result.get(), i, newref( item ) );
        }
    }
    else
    {
        for( Py_ssize_t i = 0; i < size; ++i )
        {
            PyObject* item = PyList_GET_ITEM( value, i );
            item = Member_FullValidate( im, atom, name, item );
            if( !item )
            {
                VALIDATION_ERROR;
            }
            PyList_SET_ITEM( result.get(), i, item );
        }
    }
    return result.release();
}


class AtomListFactory
{
public:
    PyObject* operator()( Member* member,
                          CAtom* atom,
                          Member* validator,
                          Py_ssize_t size )
    {
        // return AtomList_New( size, atom, validator );
        return PyList_New( size );
    }
};


class AtomCListFactory
{
public:
    PyObject* operator()( Member* member,
                          CAtom* atom,
                          Member* validator,
                          Py_ssize_t size )
    {
        // return AtomCList_New( size, atom, validator, member );
        return PyList_New( size );
    }
};


static PyObject* list_handler( Member* member,
                               CAtom* atom,
                               PyStringObject* name,
                               PyObject* value )
{
    return common_list_handler<AtomListFactory>( member, atom, name, value );
}


static PyObject* container_list_handler( Member* member,
                                         CAtom* atom,
                                         PyStringObject* name,
                                         PyObject* value )
{
    return common_list_handler<AtomCListFactory>( member, atom, name, value );
}


static PyObject* _dict_key_handler( Member* member,
                                    CAtom* atom,
                                    PyStringObject* name,
                                    PyObject* value,
                                    Member* km )
{
    PyDictPtr result( PyDict_New() );
    if( !result )
    {
        return 0; // not a validation failure
    }
    PyObject* key;
    PyObject* val;
    Py_ssize_t pos = 0;
    while( PyDict_Next( value, &pos, &key, &val ) )
    {
        PyObjectPtr vkey( Member_FullValidate( km, atom, name, key ) );
        if( !vkey )
        {
            VALIDATION_ERROR;
        }
        PyObjectPtr vval( newref( val ) );
        if( !result.set_item( vkey, vval ) )
        {
            return 0; // not a validation failure
        }
    }
    return result.release();
}


static PyObject* _dict_val_handler( Member* member,
                                    CAtom* atom,
                                    PyStringObject* name,
                                    PyObject* value,
                                    Member* vm )
{
    PyDictPtr result( PyDict_New() );
    if( !result )
    {
        return 0; // not a validation failure
    }
    PyObject* key;
    PyObject* val;
    Py_ssize_t pos = 0;
    while( PyDict_Next( value, &pos, &key, &val ) )
    {
        PyObjectPtr vval( Member_FullValidate( vm, atom, name, val ) );
        if( !vval )
        {
            VALIDATION_ERROR;
        }
        PyObjectPtr vkey( newref( key ) );
        if( !result.set_item( vkey, vval ) )
        {
            return 0; // not a validation failure
        }
    }
    return result.release();
}


static PyObject* _dict_key_val_handler( Member* member,
                                        CAtom* atom,
                                        PyStringObject* name,
                                        PyObject* value,
                                        Member* km,
                                        Member* vm )
{
    PyDictPtr result( PyDict_New() );
    if( !result )
    {
        return 0; // not a validation failure
    }
    PyObject* key;
    PyObject* val;
    Py_ssize_t pos = 0;
    while( PyDict_Next( value, &pos, &key, &val ) )
    {
        PyObjectPtr vkey( Member_FullValidate( km, atom, name, key ) );
        if( !vkey )
        {
            VALIDATION_ERROR;
        }
        PyObjectPtr vval( Member_FullValidate( vm, atom, name, val ) );
        if( !vval )
        {
            VALIDATION_ERROR;
        }
        if( !result.set_item( vkey, vval ) )
        {
            return 0; // not a validation failure
        }
    }
    return result.release();
}


static PyObject* dict_handler( Member* member,
                               CAtom* atom,
                               PyStringObject* name,
                               PyObject* value )
{
    if( !PyDict_Check( value ) )
    {
        VALIDATION_ERROR;
    }
    PyObject* km = PyTuple_GET_ITEM( member->validate_context, 0 );
    PyObject* vm = PyTuple_GET_ITEM( member->validate_context, 1 );
    if( km != Py_None && vm != Py_None )
    {
        return _dict_key_val_handler( member, atom, name, value, km, vm );
    }
    if( km != Py_None )
    {
        return _dict_key_handler( member, atom, name, value, km );
    }
    if( vm != Py_None )
    {
        return _dict_val_handler( member, atom, name, value, vm );
    }
    return PyDict_Copy( value );
}


static PyObject* instance_handler( Member* member,
                                   CAtom* atom,
                                   PyStringObject* name,
                                   PyObject* value )
{
    if( value == Py_None )
    {
        return newref( value );
    }
    if( PyObject_IsInstance( value, member->validate_context ) == 1 )
    {
        return newref( value );
    }
    VALIDATION_ERROR;
}


static PyObject* typed_handler( Member* member,
                                CAtom* atom,
                                PyStringObject* name,
                                PyObject* value )
{
    if( value == Py_None )
    {
        return newref( value );
    }
    if( PyObject_TypeCheck( value, ( PyTypeObject* )member->validate_context ) )
    {
        return newref( value );
    }
    VALIDATION_ERROR;
}


static PyObject* subclass_handler( Member* member,
                                   CAtom* atom,
                                   PyStringObject* name,
                                   PyObject* value )
{
    if( PyObject_IsSubclass( value, member->validate_context ) == 1 )
    {
        return newref( value );
    }
    VALIDATION_ERROR;
}


static PyObject* enum_handler( Member* member,
                               CAtom* atom,
                               PyStringObject* name,
                               PyObject* value )
{
    if( PySequence_Contains( member->validate_context, value ) == 1 )
    {
        return newref( value );
    }
    VALIDATION_ERROR;
}


static PyObject* callable_handler( Member* member,
                                   CAtom* atom,
                                   PyStringObject* name,
                                   PyObject* value )
{
    if( value == Py_None || PyCallable_Check( value ) )
    {
        return newref( value );
    }
    VALIDATION_ERROR;
}


static PyObject* range_handler( Member* member,
                                CAtom* atom,
                                PyStringObject* name,
                                PyObject* value )
{
    // XXX handle longs
    if( !PyInt_Check( value ) )
    {
        VALIDATION_ERROR;
    }
    PyObject* low = PyTuple_GET_ITEM( member->validate_context, 0 );
    PyObject* high = PyTuple_GET_ITEM( member->validate_context, 1 );
    if( low != Py_None )
    {
        if( PyInt_AS_LONG( low ) > PyInt_AS_LONG( value ) )
        {
            VALIDATION_ERROR;
        }
    }
    if( high != Py_None )
    {
        if( PyInt_AS_LONG( high ) < PyInt_AS_LONG( value ) )
        {
            VALIDATION_ERROR;
        }
    }
    return newref( value );
}


static PyObject* float_range_handler( Member* member,
                                      CAtom* atom,
                                      PyStringObject* name,
                                      PyObject* value )
{
    if( !PyFloat_Check( value ) )
    {
        VALIDATION_ERROR;
    }
    PyObject* low = PyTuple_GET_ITEM( member->validate_context, 0 );
    PyObject* high = PyTuple_GET_ITEM( member->validate_context, 1 );
    if( low != Py_None )
    {
        if( PyFloat_AS_DOUBLE( low ) > PyFloat_AS_DOUBLE( value ) )
        {
            VALIDATION_ERROR;
        }
    }
    if( high != Py_None )
    {
        if( PyFloat_AS_DOUBLE( high ) < PyFloat_AS_DOUBLE( value ) )
        {
            VALIDATION_ERROR;
        }
    }
    return newref( value );
}


static PyObject* coerced_handler( Member* member,
                                  CAtom* atom,
                                  PyStringObject* name,
                                  PyObject* value )
{
    PyObject* type = PyTuple_GET_ITEM( member->validate_context, 0 );
    int res = PyObject_IsInstance( value, type );
    if( res == 1 )
    {
        return newref( value );
    }
    if( res == -1 )
    {
        VALIDATION_ERROR;
    }
    PyObjectPtr args( PyTuple_New( 1 ) );
    if( !args )
    {
        return 0;
    }
    PyTuple_SET_ITEM( args.get(), 0, newref( value ) );
    PyObject* coercer = PyTuple_GET_ITEM( member->validate_context, 1 );
    PyObjectPtr coerced( PyObject_Call( coercer, args.get(), 0 ) );
    if( !coerced )
    {
        VALIDATION_ERROR;
    }
    if( PyObject_IsInstance( coerced.get(), type ) == 1 )
    {
        return coerced.release();
    }
    VALIDATION_ERROR;
}


static PyObject* object_method_handler( Member* member,
                                        CAtom* atom,
                                        PyStringObject* name,
                                        PyObject* value )
{
    PyObjectPtr method(
        PyObject_GetAttr( ( PyObject* )atom, member->validate_context ) );
    if( !method )
    {
        return 0;
    }
    PyObjectPtr args( PyTuple_New( 1 ) );
    if( !args )
    {
        return 0;
    }
    PyTuple_SET_ITEM( args.get(), 0, newref( value ) );
    return PyObject_Call( method.get(), args.get(), 0 );
}


static PyObject* member_method_handler( Member* member,
                                        CAtom* atom,
                                        PyStringObject* name,
                                        PyObject* value )
{
    PyObjectPtr method(
        PyObject_GetAttr( ( PyObject* )member, member->validate_context ) );
    if( !method )
    {
        return 0;
    }
    PyObjectPtr args( PyTuple_New( 3 ) );
    if( !args )
    {
        return 0;
    }
    PyTuple_SET_ITEM( args.get(), 0, newref( ( PyObject* )atom ) );
    PyTuple_SET_ITEM( args.get(), 1, newref( ( PyObject* )name ) );
    PyTuple_SET_ITEM( args.get(), 2, newref( value ) );
    return PyObject_Call( method.get(), args.get(), 0 );
}


// deprecated
static PyObject* str_handler( Member* member,
                              CAtom* atom,
                              PyStringObject* name,
                              PyObject* value )
{
    if( PyString_Check( value ) )
    {
        return newref( value );
    }
    VALIDATION_ERROR;
}


// deprecated
static PyObject* str_promote_handler( Member* member,
                                      CAtom* atom,
                                      PyStringObject* name,
                                      PyObject* value )
{
    if( PyString_Check( value ) )
    {
        return newref( value );
    }
    if( PyUnicode_Check( value ) )
    {
        PyObject* result = PyUnicode_AsUTF8String( value );
        if( result )
        {
            return result;
        }
    }
    VALIDATION_ERROR;
}


// deprecated
static PyObject* unicode_handler( Member* member,
                                  CAtom* atom,
                                  PyStringObject* name,
                                  PyObject* value )
{
    if( PyUnicode_Check( value ) )
    {
        return newref( value );
    }
    VALIDATION_ERROR;
}


// deprecated
static PyObject* unicode_promote_handler( Member* member,
                                          CAtom* atom,
                                          PyStringObject* name,
                                          PyObject* value )
{
    if( PyUnicode_Check( value ) )
    {
        return newref( value );
    }
    if( PyString_Check( value ) )
    {
        PyObject* result = PyUnicode_FromString( PyString_AS_STRING( value ) );
        if( result )
        {
            return result;
        }
    }
    VALIDATION_ERROR;
}


typedef PyObject* ( *ValidateHandler )( Member* member,
                                        CAtom* atom,
                                        PyStringObject* name,
                                        PyObject* value );


static ValidateHandler vld_handlers[] = {no_op_handler,
                                         bool_handler,
                                         int_handler,
                                         int_cast_handler,
                                         float_handler,
                                         float_cast_handler,
                                         bytes_handler,
                                         string_handler,
                                         tuple_handler,
                                         list_handler,
                                         container_list_handler,
                                         dict_handler,
                                         instance_handler,
                                         typed_handler,
                                         subclass_handler,
                                         enum_handler,
                                         callable_handler,
                                         range_handler,
                                         float_range_handler,
                                         coerced_handler,
                                         object_method_handler,
                                         member_method_handler,
                                         str_handler,
                                         str_promote_handler,
                                         unicode_handler,
                                         unicode_promote_handler};


// new ref on success, null and exception on failure
PyObject* Member_Validate( Member* member,
                           CAtom* atom,
                           PyStringObject* name,
                           PyObject* value )
{
    return vld_handlers[member->validate_mode]( member, atom, name, value );
}
