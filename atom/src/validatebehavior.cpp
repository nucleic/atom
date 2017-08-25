/*-----------------------------------------------------------------------------
| Copyright (c) 2013, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include <limits>
#include "member.h"
#include "atomlist.h"


using namespace PythonHelpers;


bool
Member::check_context( Validate::Mode mode, PyObject* context )
{
    switch( mode )
    {
        case Validate::Tuple:
        case Validate::List:
        case Validate::ContainerList:
            if( context != Py_None && !Member::TypeCheck( context ) )
            {
                py_expected_type_fail( context, "Member or None" );
                return false;
            }
            break;
        case Validate::Dict:
        {
            if( !PyTuple_Check( context ) )
            {
                py_expected_type_fail( context, "2-tuple of Member or None" );
                return false;
            }
            if( PyTuple_GET_SIZE( context ) != 2 )
            {
                py_expected_type_fail( context, "2-tuple of Member or None" );
                return false;
            }
            PyObject* k = PyTuple_GET_ITEM( context, 0 );
            PyObject* v = PyTuple_GET_ITEM( context, 1 );
            if( k != Py_None && !Member::TypeCheck( k ) )
            {
                py_expected_type_fail( context, "2-tuple of Member or None" );
                return false;
            }
            if( v != Py_None && !Member::TypeCheck( v ) )
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
        case Validate::FloatRange:
        {
            if( !PyTuple_Check( context ) )
            {
                py_expected_type_fail( context, "2-tuple of float or None" );
                return false;
            }
            if( PyTuple_GET_SIZE( context ) != 2 )
            {
                py_expected_type_fail( context, "2-tuple of float or None" );
                return false;
            }
            PyObject* start = PyTuple_GET_ITEM( context, 0 );
            PyObject* end = PyTuple_GET_ITEM( context, 1 );
            if( start != Py_None && !PyFloat_Check( start ) )
            {
                py_expected_type_fail( context, "2-tuple of float or None" );
                return false;
            }
            if( end != Py_None && !PyFloat_Check( end ) )
            {
                py_expected_type_fail( context, "2-tuple of float or None" );
                return false;
            }
            break;
        }
        case Validate::Range:
        {
            if( !PyTuple_Check( context ) )
            {
                py_expected_type_fail( context, "2-tuple of int or None" );
                return false;
            }
            if( PyTuple_GET_SIZE( context ) != 2 )
            {
                py_expected_type_fail( context, "2-tuple of int or None" );
                return false;
            }
            PyObject* start = PyTuple_GET_ITEM( context, 0 );
            PyObject* end = PyTuple_GET_ITEM( context, 1 );
            if( start != Py_None && !PyLong_Check( start ) )
            {
                py_expected_type_fail( context, "2-tuple of int or None" );
                return false;
            }
            if( end != Py_None && !PyLong_Check( end ) )
            {
                py_expected_type_fail( context, "2-tuple of int or None" );
                return false;
            }
            break;
        }
        case Validate::Coerced:
        {
            if( !PyTuple_Check( context ) )
            {
                py_expected_type_fail( context, "2-tuple of (type, callable)" );
                return false;
            }
            if( PyTuple_GET_SIZE( context ) != 2 )
            {
                py_expected_type_fail( context, "2-tuple of (type, callable)" );
                return false;
            }
            PyObject* type = PyTuple_GET_ITEM( context, 0 );
            PyObject* coercer = PyTuple_GET_ITEM( context, 1 );
            // XXX validate type as valid for isinstance(..., type)
            if( !PyCallable_Check( coercer ) )
            {
                py_expected_type_fail( context, "2-tuple of (type, callable)" );
                return false;
            }
            break;
        }
        case Validate::Delegate:
            if( !Member::TypeCheck( context ) )
            {
                py_expected_type_fail( context, "Member" );
                return false;
            }
            break;
        case Validate::ObjectMethod_OldNew:
        case Validate::ObjectMethod_NameOldNew:
        case Validate::MemberMethod_ObjectOldNew:
            if( !PyBytes_Check( context ) )
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


static PyObject*
validate_type_fail( Member* member, CAtom* atom, PyObject* newvalue, const char* type )
{
    PyErr_Format(
        PyExc_TypeError,
        "The '%s' member on the '%s' object must be of type '%s'. "
        "Got object of type '%s' instead.",
        _PyUnicode_AsString( member->name ),
        pyobject_cast( atom )->ob_type->tp_name,
        type,
        newvalue->ob_type->tp_name
    );
    return 0;
}


static PyObject*
no_op_handler( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    return newref( newvalue );
}


static PyObject*
bool_handler( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    if( newvalue == Py_True || newvalue == Py_False )
        return newref( newvalue );
    return validate_type_fail( member, atom, newvalue, "bool" );
}


static PyObject*
int_handler( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    if( PyLong_Check( newvalue ) )
        return newref( newvalue );
    return validate_type_fail( member, atom, newvalue, "int" );
}

static PyObject*
int_promote_handler( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    if( PyInt_Check( newvalue ) )
        return newref( newvalue );
    if( PyFloat_Check( newvalue ) ) {
        double value = PyFloat_AS_DOUBLE( newvalue );
        if( value < static_cast<double>( std::numeric_limits<long>::min() ) ||
            value > static_cast<double>( std::numeric_limits<long>::max() ) )
        {
            PyErr_SetString( PyExc_OverflowError, "Python float too large to convert to C long" );
            return 0;
        }
        return PyInt_FromLong( static_cast<long>( value ) );
    }
    if( PyLong_Check( newvalue ) ) {
        long value = PyLong_AsLong( newvalue );
        if( value == -1 && PyErr_Occurred() )
            return 0;
        return PyInt_FromLong( value );
    }
    return validate_type_fail( member, atom, newvalue, "int float or long" );
}


static PyObject*
long_handler( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    if( PyLong_Check( newvalue ) )
        return newref( newvalue );
    return validate_type_fail( member, atom, newvalue, "long" );
}


static PyObject*
long_promote_handler( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    if( PyLong_Check( newvalue ) )
        return newref( newvalue );
    #if PY_MAJOR_VERSION < 3
        if( PyInt_Check( newvalue ) )
            return PyLong_FromLong( PyInt_AS_LONG( newvalue ) );
    #endif
    return validate_type_fail( member, atom, newvalue, "long" );
}


static PyObject*
float_handler( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    if( PyFloat_Check( newvalue ) )
        return newref( newvalue );
    return validate_type_fail( member, atom, newvalue, "float" );
}


static PyObject*
float_promote_handler( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    if( PyFloat_Check( newvalue ) )
        return newref( newvalue );
    #if PY_MAJOR_VERSION < 3
    if( PyInt_Check( newvalue ) )
        return PyFloat_FromDouble( static_cast<double>( PyInt_AS_LONG( newvalue ) ) );
    #endif
    if( PyLong_Check( newvalue ) )
    {
        double value = PyLong_AsDouble( newvalue );
        if( value == -1.0 && PyErr_Occurred() )
            return 0;
        return PyFloat_FromDouble( value );
    }
    return validate_type_fail( member, atom, newvalue, "float" );
}


static PyObject*
str_handler( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    if( PyBytes_Check( newvalue ) )
        return newref( newvalue );
    return validate_type_fail( member, atom, newvalue, "str" );
}


static PyObject*
str_promote_handler( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    if( PyBytes_Check( newvalue ) )
        return newref( newvalue );
    if( PyUnicode_Check( newvalue ) )
        return PyUnicode_AsUTF8String( newvalue );
    return validate_type_fail( member, atom, newvalue, "str" );
}


static PyObject*
unicode_handler( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    if( PyUnicode_Check( newvalue ) )
        return newref( newvalue );
    return validate_type_fail( member, atom, newvalue, "unicode" );
}


static PyObject*
unicode_promote_handler( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    if( PyUnicode_Check( newvalue ) )
        return newref( newvalue );
    if( PyBytes_Check( newvalue ) )
        return PyUnicode_FromString( PyBytes_AsString(newvalue) );
    return validate_type_fail( member, atom, newvalue, "unicode" );
}


static PyObject*
tuple_handler( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    if( !PyTuple_Check( newvalue ) )
        return validate_type_fail( member, atom, newvalue, "tuple" );
    PyTuplePtr tupleptr( newref( newvalue ) );
    if( member->validate_context != Py_None )
    {
        Py_ssize_t size = PyTuple_GET_SIZE( newvalue );
        PyTuplePtr tuplecopy = PyTuple_New( size );
        if( !tuplecopy )
            return 0;
        Member* item_member = member_cast( member->validate_context );
        for( Py_ssize_t i = 0; i < size; ++i )
        {
            PyObjectPtr item( tupleptr.get_item( i ) );
            PyObjectPtr valid_item( item_member->full_validate( atom, Py_None, item.get() ) );
            if( !valid_item )
                return 0;
            tuplecopy.set_item( i, valid_item );
        }
        tupleptr = tuplecopy;
    }
    return tupleptr.release();
}


template<typename ListFactory> PyObject*
common_list_handler( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    if( !PyList_Check( newvalue ) )
        return validate_type_fail( member, atom, newvalue, "list" );
    Member* validator = 0;
    if( member->validate_context != Py_None )
        validator = member_cast( member->validate_context );
    Py_ssize_t size = PyList_GET_SIZE( newvalue );
    PyListPtr listptr( ListFactory()( member, atom, validator, size ) );
    if( !listptr )
        return 0;
    if( !validator )
    {
        for( Py_ssize_t i = 0; i < size; ++i )
            listptr.set_item( i, newref( PyList_GET_ITEM( newvalue, i ) ) );
    }
    else
    {
        for( Py_ssize_t i = 0; i < size; ++i )
        {
            PyObject* item = PyList_GET_ITEM( newvalue, i );
            PyObjectPtr valid_item( validator->full_validate( atom, Py_None, item ) );
            if( !valid_item )
                return 0;
            listptr.set_item( i, valid_item );
        }
    }
    return listptr.release();
}


class AtomListFactory
{
public:
    PyObject* operator()( Member* member, CAtom* atom, Member* validator, Py_ssize_t size )
    {
        return AtomList_New( size, atom, validator );
    }
};


class AtomCListFactory
{
public:
    PyObject* operator()( Member* member, CAtom* atom, Member* validator, Py_ssize_t size )
    {
        return AtomCList_New( size, atom, validator, member );
    }
};


static PyObject*
list_handler( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    return common_list_handler<AtomListFactory>( member, atom, oldvalue, newvalue );
}


static PyObject*
container_list_handler( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    return common_list_handler<AtomCListFactory>( member, atom, oldvalue, newvalue );
}


static PyObject*
validate_dict_key_value( Member* keymember, Member* valmember, CAtom* atom, PyObject* dict )
{
    PyObject* key;
    PyObject* value;
    Py_ssize_t pos = 0;
    PyDictPtr newptr( PyDict_New() );
    if( !newptr )
        return 0;
    while( PyDict_Next( dict, &pos, &key, &value ) )
    {
        PyObjectPtr keyptr( keymember->full_validate( atom, Py_None, key ) );
        if( !keyptr )
            return 0;
        PyObjectPtr valptr( valmember->full_validate( atom, Py_None, value ) );
        if( !valptr )
            return 0;
        if( !newptr.set_item( keyptr, valptr ) )
            return 0;
    }
    return newptr.release();
}


static PyObject*
validate_dict_value( Member* valmember, CAtom* atom, PyObject* dict )
{
    PyObject* key;
    PyObject* value;
    Py_ssize_t pos = 0;
    PyDictPtr newptr( PyDict_New() );
    if( !newptr )
        return 0;
    while( PyDict_Next( dict, &pos, &key, &value ) )
    {
        PyObjectPtr keyptr( newref( key ) );
        PyObjectPtr valptr( valmember->full_validate( atom, Py_None, value ) );
        if( !valptr )
            return 0;
        if( !newptr.set_item( keyptr, valptr ) )
            return 0;
    }
    return newptr.release();
}


static PyObject*
validate_dict_key( Member* keymember, CAtom* atom, PyObject* dict )
{
    PyObject* key;
    PyObject* value;
    Py_ssize_t pos = 0;
    PyDictPtr newptr( PyDict_New() );
    if( !newptr )
        return 0;
    while( PyDict_Next( dict, &pos, &key, &value ) )
    {
        PyObjectPtr keyptr( keymember->full_validate( atom, Py_None, key ) );
        if( !keyptr )
            return 0;
        PyObjectPtr valptr( newref( value ) );
        if( !newptr.set_item( keyptr, valptr ) )
            return 0;
    }
    return newptr.release();
}


static PyObject*
dict_handler( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    if( !PyDict_Check( newvalue ) )
        return validate_type_fail( member, atom, newvalue, "dict" );
    PyObject* k = PyTuple_GET_ITEM( member->validate_context, 0 );
    PyObject* v = PyTuple_GET_ITEM( member->validate_context, 1 );
    if( k != Py_None && v != Py_None )
        return validate_dict_key_value( member_cast( k ), member_cast( v ), atom, newvalue );
    if( v != Py_None )
        return validate_dict_value( member_cast( v ), atom, newvalue );
    if( k != Py_None )
        return validate_dict_key( member_cast( k ), atom, newvalue );
    return PyDict_Copy( newvalue );
}


static PyObject*
instance_handler( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    if( newvalue == Py_None )
        return newref( newvalue );
    int res = PyObject_IsInstance( newvalue, member->validate_context );
    if( res < 0 )
        return 0;
    if( res == 1 )
        return newref( newvalue );
    return py_type_fail( "invalid instance type" );
}


static PyObject*
typed_handler( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    if( newvalue == Py_None )
        return newref( newvalue );
    PyTypeObject* type = pytype_cast( member->validate_context );
    if( PyObject_TypeCheck( newvalue, type ) )
        return newref( newvalue );
    return validate_type_fail( member, atom, newvalue, type->tp_name );
}


static PyObject*
subclass_handler( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    int res = PyObject_IsSubclass( newvalue, member->validate_context );
    if( res < 0 )
        return 0;
    if( res == 1 )
        return newref( newvalue );
    return py_type_fail( "invalid subclass type" );
}


static PyObject*
enum_handler( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    int res = PySequence_Contains( member->validate_context, newvalue );
    if( res < 0 )
        return 0;
    if( res == 1 )
        return newref( newvalue );
    return py_value_fail( "invalid enum value" );
}


static PyObject*
callable_handler( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    if( newvalue == Py_None )
        return newref( newvalue );
    if( PyCallable_Check( newvalue ) )
        return newref( newvalue );
    return validate_type_fail( member, atom, newvalue, "callable" );
}


static PyObject*
float_range_handler( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    if( !PyFloat_Check( newvalue ) )
        return validate_type_fail( member, atom, newvalue, "float" );
    PyObject* low = PyTuple_GET_ITEM( member->validate_context, 0 );
    PyObject* high = PyTuple_GET_ITEM( member->validate_context, 1 );
    double value = PyFloat_AS_DOUBLE( newvalue );
    if( low != Py_None )
    {
        if( PyFloat_AS_DOUBLE( low ) > value )
            return py_type_fail( "range value too small" );
    }
    if( high != Py_None )
    {
        if( PyFloat_AS_DOUBLE( high ) < value )
            return py_type_fail( "range value too large" );
    }
    return newref( newvalue );
}


static PyObject*
range_handler( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    if( !PyLong_Check( newvalue ) )
        return validate_type_fail( member, atom, newvalue, "int" );
    PyObject* low = PyTuple_GET_ITEM( member->validate_context, 0 );
    PyObject* high = PyTuple_GET_ITEM( member->validate_context, 1 );
    long value = PyLong_AS_LONG( newvalue );
    if( low != Py_None )
    {
        if( PyLong_AS_LONG( low ) > value )
            return py_type_fail( "range value too small" );
    }
    if( high != Py_None )
    {
        if( PyLong_AS_LONG( high ) < value )
            return py_type_fail( "range value too large" );
    }
    return newref( newvalue );
}


static PyObject*
coerced_handler( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    PyObject* type = PyTuple_GET_ITEM( member->validate_context, 0 );
    int res = PyObject_IsInstance( newvalue, type );
    if( res == 1 )
        return newref( newvalue );
    if( res == -1 )
        return 0;
    PyTuplePtr argsptr( PyTuple_New( 1 ) );
    if( !argsptr )
        return 0;
    argsptr.initialize( 0, newref( newvalue ) );
    PyObject* coercer = PyTuple_GET_ITEM( member->validate_context, 1 );
    PyObjectPtr callable( newref( coercer ) );
    PyObjectPtr coerced( callable( argsptr ) );
    if( !coerced )
        return 0;
    res = PyObject_IsInstance( coerced.get(), type );
    if( res == 1 )
        return coerced.release();
    if( res == -1 )
        return 0;
    return py_type_fail( "could not coerce value to an appropriate type" );
}


static PyObject*
delegate_handler( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    Member* delegate = member_cast( member->validate_context );
    return delegate->validate( atom, oldvalue, newvalue );
}


static PyObject*
object_method_old_new_handler(
    Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    PyObjectPtr callable( PyObject_GetAttr( pyobject_cast( atom ), member->validate_context ) );
    if( !callable )
        return 0;
    PyTuplePtr args( PyTuple_New( 2 ) );
    if( !args )
        return 0;
    args.initialize( 0, newref( oldvalue ) );
    args.initialize( 1, newref( newvalue ) );
    return callable( args ).release();
}


static PyObject*
object_method_name_old_new_handler(
    Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    PyObjectPtr callable( PyObject_GetAttr( pyobject_cast( atom ), member->validate_context ) );
    if( !callable )
        return 0;
    PyTuplePtr args( PyTuple_New( 3 ) );
    if( !args )
        return 0;
    args.initialize( 0, newref( member->name ) );
    args.initialize( 1, newref( oldvalue ) );
    args.initialize( 2, newref( newvalue ) );
    return callable( args ).release();
}


static PyObject*
member_method_object_old_new_handler(
    Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    PyObjectPtr callable( PyObject_GetAttr( pyobject_cast( member ), member->validate_context ) );
    if( !callable )
        return 0;
    PyTuplePtr args( PyTuple_New( 3 ) );
    if( !args )
        return 0;
    args.initialize( 0, newref( pyobject_cast( atom ) ) );
    args.initialize( 1, newref( oldvalue ) );
    args.initialize( 2, newref( newvalue ) );
    return callable( args ).release();
}


typedef PyObject*
( *handler )( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue );


static handler
handlers[] = {
    no_op_handler,
    bool_handler,
    int_handler,
    int_promote_handler,
    long_handler,
    long_promote_handler,
    float_handler,
    float_promote_handler,
    str_handler,
    str_promote_handler,
    unicode_handler,
    unicode_promote_handler,
    tuple_handler,
    list_handler,
    container_list_handler,
    dict_handler,
    instance_handler,
    typed_handler,
    subclass_handler,
    enum_handler,
    callable_handler,
    float_range_handler,
    range_handler,
    coerced_handler,
    delegate_handler,
    object_method_old_new_handler,
    object_method_name_old_new_handler,
    member_method_object_old_new_handler
};


PyObject*
Member::validate( CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    if( get_validate_mode() >= sizeof( handlers ) )
        return no_op_handler( this, atom, oldvalue, newvalue );
    return handlers[ get_validate_mode() ]( this, atom, oldvalue, newvalue );
}
