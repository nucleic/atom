/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2021, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file LICENSE, distributed with this software.
|----------------------------------------------------------------------------*/
#include <limits>
#include <iostream>
#include <sstream>
#include <cppy/cppy.h>
#include "member.h"
#include "atomlist.h"
#include "atomdict.h"
#include "atomset.h"


namespace atom
{


namespace
{

bool validate_type_tuple_types( PyObject* type_tuple_types )
{
    if( PyTuple_Check( type_tuple_types ) )
    {
        Py_ssize_t len = PySequence_Size( type_tuple_types );
        for( Py_ssize_t i = 0; i < len; i++ )
        {
            if( !PyType_Check( PyTuple_GET_ITEM( type_tuple_types, i ) ) )
            {
                PyErr_Format(
                    PyExc_TypeError,
                    "Expected type or tuple of types. Got a tuple containing an instance of `%s` instead.",
                    Py_TYPE( PyTuple_GET_ITEM( type_tuple_types, i ) )->tp_name
                );
                return false;
            }
        }
        return true;
    }
    if( !PyType_Check( type_tuple_types ) )
    {
        cppy::type_error( type_tuple_types, "type or tuple of types" );
        return false;
    }
    return true;
}

}  // namespace


bool
Member::check_context( Validate::Mode mode, PyObject* context )
{
    switch( mode )
    {
        case Validate::Tuple:
        case Validate::List:
        case Validate::ContainerList:
        case Validate::Set:
            if( context != Py_None && !Member::TypeCheck( context ) )
            {
                cppy::type_error( context, "Member or None" );
                return false;
            }
            break;
        case Validate::Dict:
        {
            if( !PyTuple_Check( context ) )
            {
                cppy::type_error( context, "2-tuple of Member or None" );
                return false;
            }
            if( PyTuple_GET_SIZE( context ) != 2 )
            {
                cppy::type_error( context, "2-tuple of Member or None" );
                return false;
            }
            PyObject* k = PyTuple_GET_ITEM( context, 0 );
            PyObject* v = PyTuple_GET_ITEM( context, 1 );
            if( k != Py_None && !Member::TypeCheck( k ) )
            {
                cppy::type_error( context, "2-tuple of Member or None" );
                return false;
            }
            if( v != Py_None && !Member::TypeCheck( v ) )
            {
                cppy::type_error( context, "2-tuple of Member or None" );
                return false;
            }
            break;
        }
        case Validate::DefaultDict:
        {
            if( !PyTuple_Check( context ) )
            {
                cppy::type_error( context, "3-tuple: Member|None, Member|None, Callable[[], Any]" );
                return false;
            }
            if( PyTuple_GET_SIZE( context ) != 3 )
            {
                cppy::type_error( context, "3-tuple: Member|None, Member|None, Callable[[], Any]" );
                return false;
            }
            PyObject* k = PyTuple_GET_ITEM( context, 0 );
            PyObject* v = PyTuple_GET_ITEM( context, 1 );
            PyObject* f = PyTuple_GET_ITEM( context, 2 );
            if( k != Py_None && !Member::TypeCheck( k ) )
            {
                cppy::type_error( context, "3-tuple: Member|None, Member|None, Callable[[], Any]" );
                return false;
            }
            if( v != Py_None && !Member::TypeCheck( v ) )
            {
                cppy::type_error( context, "3-tuple: Member|None, Member|None, Callable[[], Any]" );
                return false;
            }
            if( PyCallable_Check( f ) == 0)
            {
                cppy::type_error( context, "3-tuple: Member|None, Member|None, Callable[[], Any]" );
                return false;
            }
            break;
        }
        case Validate::OptionalInstance:
        case Validate::Instance:
        case Validate::Subclass:
        {
            return validate_type_tuple_types( context );
        }
        case Validate::OptionalTyped:
        case Validate::Typed:
            if( !PyType_Check( context ) )
            {
                cppy::type_error( context, "type" );
                return false;
            }
            break;
        case Validate::Enum:
            if( !PySequence_Check( context ) )
            {
                cppy::type_error( context, "sequence" );
                return false;
            }
            break;
        case Validate::FloatRange:
        {
            if( !PyTuple_Check( context ) )
            {
                cppy::type_error( context, "2-tuple of float or None" );
                return false;
            }
            if( PyTuple_GET_SIZE( context ) != 2 )
            {
                cppy::type_error( context, "2-tuple of float or None" );
                return false;
            }
            PyObject* start = PyTuple_GET_ITEM( context, 0 );
            PyObject* end = PyTuple_GET_ITEM( context, 1 );
            if( start != Py_None && !PyFloat_Check( start ) )
            {
                cppy::type_error( context, "2-tuple of float or None" );
                return false;
            }
            if( end != Py_None && !PyFloat_Check( end ) )
            {
                cppy::type_error( context, "2-tuple of float or None" );
                return false;
            }
            break;
        }
        case Validate::Range:
        {
            if( !PyTuple_Check( context ) )
            {
                cppy::type_error( context, "2-tuple of int or None" );
                return false;
            }
            if( PyTuple_GET_SIZE( context ) != 2 )
            {
                cppy::type_error( context, "2-tuple of int or None" );
                return false;
            }
            PyObject* start = PyTuple_GET_ITEM( context, 0 );
            PyObject* end = PyTuple_GET_ITEM( context, 1 );
            if( start != Py_None && !PyLong_Check( start ) )
            {
                cppy::type_error( context, "2-tuple of int or None" );
                return false;
            }
            if( end != Py_None && !PyLong_Check( end ) )
            {
                cppy::type_error( context, "2-tuple of int or None" );
                return false;
            }
            break;
        }
        case Validate::Coerced:
        {
            if( !PyTuple_Check( context ) )
            {
                cppy::type_error( context, "2-tuple of (type, callable)" );
                return false;
            }
            if( PyTuple_GET_SIZE( context ) != 2 )
            {
                PyErr_Format(
                    PyExc_TypeError,
                    "Expected 2-tuple of (type, callable). Got a tuple of length %d instead.",
                    PyTuple_GET_SIZE( context )
                );
                return false;
            }
            PyObject* type = PyTuple_GET_ITEM( context, 0 );
            PyObject* coercer = PyTuple_GET_ITEM( context, 1 );
            if( !validate_type_tuple_types( type ) )
            {
                return false;
            }
            if( !PyCallable_Check( coercer ) )
            {
                cppy::type_error( context, "2-tuple of (type, callable)" );
                return false;
            }
            break;
        }
        case Validate::Delegate:
            if( !Member::TypeCheck( context ) )
            {
                cppy::type_error( context, "Member" );
                return false;
            }
            break;
        case Validate::ObjectMethod_OldNew:
        case Validate::ObjectMethod_NameOldNew:
        case Validate::MemberMethod_ObjectOldNew:
            if( !PyUnicode_Check( context ) )
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


namespace
{


std::string name_from_type_tuple_types( PyObject* type_tuple_types )
{
    // This should never be used if the input can be something else than a type
    // or a tuple of types.
    std::ostringstream ostr;
    if( PyType_Check( type_tuple_types ) )
    {
        PyTypeObject* type = pytype_cast( type_tuple_types );
        ostr << type->tp_name;
    }
    else
    {
        ostr << "(";
        Py_ssize_t len = PySequence_Size( type_tuple_types );
        for( Py_ssize_t i = 0; i < len; i++ )
        {
            PyTypeObject* type = pytype_cast( PyTuple_GET_ITEM( type_tuple_types, i ) );
            ostr << type->tp_name;
            if( i != len-1 )
                ostr << ", ";
        }
        ostr << ")";
    }

    return ostr.str();
}


PyObject*
validate_type_fail( Member* member, CAtom* atom, PyObject* newvalue, const char* type )
{
    PyErr_Format(
        PyExc_TypeError,
        "The '%s' member on the '%s' object must be of type '%s'. "
        "Got object of type '%s' instead.",
        PyUnicode_AsUTF8( member->name ),
        Py_TYPE( pyobject_cast( atom ) )->tp_name,
        type,
        Py_TYPE( newvalue )->tp_name
    );
    return 0;
}


PyObject*
no_op_handler( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    return cppy::incref( newvalue );
}


PyObject*
bool_handler( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    if( newvalue == Py_True || newvalue == Py_False )
    {
        return cppy::incref( newvalue );
    }
    return validate_type_fail( member, atom, newvalue, "bool" );
}


PyObject*
long_handler( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    if( PyLong_Check( newvalue ) )
    {
        return cppy::incref( newvalue );
    }
    return validate_type_fail( member, atom, newvalue, "int" );
}


PyObject*
long_promote_handler( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    if( PyLong_Check( newvalue ) )
    {
        return cppy::incref( newvalue );
    }

    if( PyFloat_Check( newvalue ) )
    {
        double value = PyFloat_AS_DOUBLE( newvalue );
        return PyLong_FromDouble( value );
    }

    return validate_type_fail( member, atom, newvalue, "int" );
}


PyObject*
float_handler( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    if( PyFloat_Check( newvalue ) )
    {
        return cppy::incref( newvalue );
    }

    return validate_type_fail( member, atom, newvalue, "float" );
}


PyObject*
float_promote_handler( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    if( PyFloat_Check( newvalue ) )
    {
        return cppy::incref( newvalue );
    }
    if( PyLong_Check( newvalue ) )
    {
        double value = PyLong_AsDouble( newvalue );
        if( value == -1.0 && PyErr_Occurred() )
        {
            return 0;
        }
        return PyFloat_FromDouble( value );
    }
    return validate_type_fail( member, atom, newvalue, "float" );
}


PyObject*
bytes_handler( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    if( PyBytes_Check( newvalue ) )
    {
        return cppy::incref( newvalue );
    }

    return validate_type_fail( member, atom, newvalue, "bytes" );
}

PyObject*
bytes_promote_handler( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    if( PyBytes_Check( newvalue ) )
    {
        return cppy::incref( newvalue );
    }

    if( PyUnicode_Check( newvalue ) )
    {
        return PyUnicode_AsUTF8String( newvalue );
    }

    return validate_type_fail( member, atom, newvalue, "bytes" );
}

PyObject*
str_handler( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    if( PyUnicode_Check( newvalue ) )
    {
        return cppy::incref( newvalue );
    }
    return validate_type_fail( member, atom, newvalue, "str" );
}


PyObject*
str_promote_handler( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    if( PyUnicode_Check( newvalue ) )
    {
        return cppy::incref( newvalue );
    }
    if( PyBytes_Check( newvalue ) )
    {
        return PyUnicode_FromString( PyBytes_AS_STRING( newvalue ) );
    }
    return validate_type_fail( member, atom, newvalue, "str" );
}


PyObject*
tuple_handler( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    if( !PyTuple_Check( newvalue ) )
    {
        return validate_type_fail( member, atom, newvalue, "tuple" );
    }
    cppy::ptr tupleptr( cppy::incref( newvalue ) );
    if( member->validate_context != Py_None )
    {
        Py_ssize_t size = PyTuple_GET_SIZE( newvalue );
        cppy::ptr tuplecopy = PyTuple_New( size );
        if( !tuplecopy )
        {
            return 0;
        }
        Member* item_member = member_cast( member->validate_context );
        for( Py_ssize_t i = 0; i < size; ++i )
        {
            cppy::ptr item( cppy::incref( PyTuple_GET_ITEM( tupleptr.get(), i ) ) );
            cppy::ptr valid_item( item_member->full_validate( atom, Py_None, item.get() ) );
            if( !valid_item )
            {
                return 0;
            }
            PyTuple_SET_ITEM( tuplecopy.get(), i, valid_item.release() );
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
    {
        validator = member_cast( member->validate_context );
    }
    Py_ssize_t size = PyList_GET_SIZE( newvalue );
    cppy::ptr listptr( ListFactory()( member, atom, validator, size ) );
    if( !listptr )
    {
        return 0;
    }
    if( !validator )
    {
        for( Py_ssize_t i = 0; i < size; ++i )
            PyList_SET_ITEM( listptr.get(), i, cppy::incref( PyList_GET_ITEM( newvalue, i ) ) );
    }
    else
    {
        for( Py_ssize_t i = 0; i < size; ++i )
        {
            PyObject* item = PyList_GET_ITEM( newvalue, i );
            cppy::ptr valid_item( validator->full_validate( atom, Py_None, item ) );
            if( !valid_item )
            {
                return 0;
            }
            PyList_SET_ITEM( listptr.get(), i, valid_item.release() );
        }
    }
    return listptr.release();
}


class AtomListFactory
{
public:
    PyObject* operator()( Member* member, CAtom* atom, Member* validator, Py_ssize_t size )
    {
        return atom::AtomList::New( size, atom, validator );
    }
};


class AtomCListFactory
{
public:
    PyObject* operator()( Member* member, CAtom* atom, Member* validator, Py_ssize_t size )
    {
        return atom::AtomCList::New( size, atom, validator, member );
    }
};


PyObject*
list_handler( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    return common_list_handler<AtomListFactory>( member, atom, oldvalue, newvalue );
}


PyObject*
container_list_handler( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    return common_list_handler<AtomCListFactory>( member, atom, oldvalue, newvalue );
}


PyObject*
set_handler( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    if( !PyAnySet_Check( newvalue ) )
        return validate_type_fail( member, atom, newvalue, "set" );

    // Get the validator if it exists
    Member* validator = 0;
    if( member->validate_context != Py_None )
    {
        validator = member_cast( member->validate_context );
    }

    // Create a new atom set and update it.
    cppy::ptr newset( atom::AtomSet::New( atom, validator ) );
    if( !newset )
    {
        return 0;
    }

    if( atom::AtomSet::Update( atomset_cast( newset.get() ), newvalue) < 0 )
    {
        return 0;
    }

    return newset.release();
}


PyObject*
dict_handler( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    if( !PyDict_Check( newvalue ) )
        return validate_type_fail( member, atom, newvalue, "dict" );

    // Get the key validator if it exists
    PyObject* k = PyTuple_GET_ITEM( member->validate_context, 0 );
    Member* key_validator = 0;
    if( k != Py_None )
    {
        key_validator = member_cast( k );
    }

    // Get the value validator if it exists
    PyObject* v = PyTuple_GET_ITEM( member->validate_context, 1 );
    Member* value_validator = 0;
    if( v != Py_None )
    {
        value_validator = member_cast( v );
    }

    // Create a new atom dict and update it.
    cppy::ptr newdict( atom::AtomDict::New( atom, key_validator, value_validator ) );
    if( !newdict )
    {
        std::cout << "Failed to create atomdict" << std::flush;
        return 0;
    }

    if( atom::AtomDict::Update( atomdict_cast( newdict.get() ), newvalue ) < 0 )
    {
        return 0;
    }

    return newdict.release();
}


PyObject*
default_dict_handler( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    if( !PyDict_Check( newvalue ) )
        return validate_type_fail( member, atom, newvalue, "dict" );

    // Get the key validator if it exists
    PyObject* k = PyTuple_GET_ITEM( member->validate_context, 0 );
    Member* key_validator = 0;
    if( k != Py_None )
    {
        key_validator = member_cast( k );
    }

    // Get the value validator if it exists
    PyObject* v = PyTuple_GET_ITEM( member->validate_context, 1 );
    Member* value_validator = 0;
    if( v != Py_None )
    {
        value_validator = member_cast( v );
    }

    // Get the value factory if it exists
    PyObject* factory = PyTuple_GET_ITEM( member->validate_context, 2 );

    // Create a new atom dict and update it.
    cppy::ptr newdict( atom::DefaultAtomDict::New( atom, key_validator, value_validator, factory) );
    if( !newdict )
    {
        std::cout << "Failed to create atomdefaultdict" << std::flush;
        return 0;
    }

    if( atom::AtomDict::Update( atomdict_cast( newdict.get() ), newvalue ) < 0 )
    {
        return 0;
    }

    return newdict.release();
}


PyObject*
non_optional_instance_handler( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    int res = PyObject_IsInstance( newvalue, member->validate_context );
    if( res < 0 )
        return 0;
    if( res == 1 )
        return cppy::incref( newvalue );
    return validate_type_fail( member, atom, newvalue, name_from_type_tuple_types( member->validate_context ).c_str() );
}


PyObject*
instance_handler( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    if( newvalue == Py_None )
        return cppy::incref( newvalue );
    return non_optional_instance_handler( member, atom, oldvalue, newvalue );
}


PyObject*
non_optional_typed_handler( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    PyTypeObject* type = pytype_cast( member->validate_context );
    if( PyObject_TypeCheck( newvalue, type ) )
        return cppy::incref( newvalue );
    return validate_type_fail( member, atom, newvalue, type->tp_name );
}


PyObject*
typed_handler( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    if( newvalue == Py_None )
        return cppy::incref( newvalue );
    return non_optional_typed_handler( member, atom, oldvalue, newvalue );
}


PyObject*
subclass_handler( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{

    if( !PyType_Check( newvalue ) )
    {
        PyErr_Format(
            PyExc_TypeError,
            "The '%s' member on the '%s' object must be a subclass of '%s'. "
            "Got instance of '%s' instead.",
            PyUnicode_AsUTF8( member->name ),
            Py_TYPE( pyobject_cast( atom ) )->tp_name,
            name_from_type_tuple_types( member->validate_context ).c_str(),
            Py_TYPE( newvalue )->tp_name
        );
        return 0;
    }

    int res = PyObject_IsSubclass( newvalue, member->validate_context );
    if( res < 0 )
        return 0;
    if( res == 1 )
        return cppy::incref( newvalue );

    if( PyType_Check( newvalue ) )
    {
        PyTypeObject* type = pytype_cast( newvalue );
        PyErr_Format(
            PyExc_TypeError,
            "The '%s' member on the '%s' object must be a subclass of '%s'. "
            "Got class '%s' instead.",
            PyUnicode_AsUTF8( member->name ),
            Py_TYPE( pyobject_cast( atom ) )->tp_name,
            name_from_type_tuple_types( member->validate_context ).c_str(),
            type->tp_name
        );
    }

    return 0;
}


PyObject*
enum_handler( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    int res = PySequence_Contains( member->validate_context, newvalue );
    if( res < 0 )
        return 0;
    if( res == 1 )
        return cppy::incref( newvalue );
    return cppy::value_error( "invalid enum value" );
}


PyObject*
callable_handler( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    if( newvalue == Py_None )
        return cppy::incref( newvalue );
    if( PyCallable_Check( newvalue ) )
        return cppy::incref( newvalue );
    return validate_type_fail( member, atom, newvalue, "callable" );
}


PyObject*
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
            return cppy::type_error( "range value too small" );
    }
    if( high != Py_None )
    {
        if( PyFloat_AS_DOUBLE( high ) < value )
            return cppy::type_error( "range value too large" );
    }
    return cppy::incref( newvalue );
}


PyObject*
float_range_promote_handler( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    if( PyFloat_Check( newvalue ) )
        return float_range_handler( member, atom, oldvalue, newvalue );
    if( PyLong_Check( newvalue ) )
    {
        double value = PyLong_AsDouble( newvalue );
        if( value == -1.0 && PyErr_Occurred() )
            return 0;
        cppy::ptr convertedvalue( PyFloat_FromDouble( value ) );
        return float_range_handler( member, atom, oldvalue, convertedvalue.get() );
    }
    return validate_type_fail( member, atom, newvalue, "float" );
}


PyObject*
range_handler( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    if( !PyLong_Check( newvalue ) )
        return validate_type_fail( member, atom, newvalue, "int" );
    PyObject* low = PyTuple_GET_ITEM( member->validate_context, 0 );
    PyObject* high = PyTuple_GET_ITEM( member->validate_context, 1 );
    if( low != Py_None )
    {
        if( PyObject_RichCompareBool( low , newvalue, Py_GT ) )
            return cppy::type_error( "range value too small" );
    }
    if( high != Py_None )
    {
        if( PyObject_RichCompareBool( high , newvalue, Py_LT ) )
            return cppy::type_error( "range value too large" );
    }
    return cppy::incref( newvalue );
}


PyObject*
coerced_handler( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    PyObject* type = PyTuple_GET_ITEM( member->validate_context, 0 );
    int res = PyObject_IsInstance( newvalue, type );
    if( res == 1 )
        return cppy::incref( newvalue );
    if( res == -1 )
        return 0;
    cppy::ptr argsptr( PyTuple_New( 1 ) );
    if( !argsptr )
        return 0;
    PyTuple_SET_ITEM(argsptr.get(), 0, cppy::incref( newvalue ) );
    PyObject* coercer = PyTuple_GET_ITEM( member->validate_context, 1 );
    cppy::ptr callable( cppy::incref( coercer ) );
    cppy::ptr coerced( callable.call( argsptr ) );
    if( !coerced )
        return 0;
    res = PyObject_IsInstance( coerced.get(), type );
    if( res == 1 )
        return coerced.release();
    if( res == -1 )
        return 0;
    return cppy::type_error( "could not coerce value to an appropriate type" );
}


PyObject*
delegate_handler( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    Member* delegate = member_cast( member->validate_context );
    return delegate->validate( atom, oldvalue, newvalue );
}


PyObject*
object_method_old_new_handler(
    Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    cppy::ptr callable( PyObject_GetAttr( pyobject_cast( atom ), member->validate_context ) );
    if( !callable )
        return 0;
    cppy::ptr args( PyTuple_New( 2 ) );
    if( !args )
        return 0;
    PyTuple_SET_ITEM(args.get(), 0, cppy::incref( oldvalue ) );
    PyTuple_SET_ITEM(args.get(), 1, cppy::incref( newvalue ) );
    return callable.call( args );
}


PyObject*
object_method_name_old_new_handler(
    Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    cppy::ptr callable( PyObject_GetAttr( pyobject_cast( atom ), member->validate_context ) );
    if( !callable )
        return 0;
    cppy::ptr args( PyTuple_New( 3 ) );
    if( !args )
        return 0;
    PyTuple_SET_ITEM(args.get(), 0, cppy::incref( member->name ) );
    PyTuple_SET_ITEM(args.get(), 1, cppy::incref( oldvalue ) );
    PyTuple_SET_ITEM(args.get(), 2, cppy::incref( newvalue ) );
    return callable.call( args );
}


PyObject*
member_method_object_old_new_handler(
    Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    cppy::ptr callable( PyObject_GetAttr( pyobject_cast( member ), member->validate_context ) );
    if( !callable )
        return 0;
    cppy::ptr args( PyTuple_New( 3 ) );
    if( !args )
        return 0;
    PyTuple_SET_ITEM(args.get(), 0, cppy::incref( pyobject_cast( atom ) ) );
    PyTuple_SET_ITEM(args.get(), 1, cppy::incref( oldvalue ) );
    PyTuple_SET_ITEM(args.get(), 2, cppy::incref( newvalue ) );
    return callable.call( args );
}


typedef PyObject*
( *handler )( Member* member, CAtom* atom, PyObject* oldvalue, PyObject* newvalue );


static handler
handlers[] = {
    no_op_handler,
    bool_handler,
    long_handler,
    long_promote_handler,
    float_handler,
    float_promote_handler,
    bytes_handler,
    bytes_promote_handler,
    str_handler,
    str_promote_handler,
    tuple_handler,
    list_handler,
    container_list_handler,
    set_handler,
    dict_handler,
    default_dict_handler,
    instance_handler,
    non_optional_instance_handler,
    typed_handler,
    non_optional_typed_handler,
    subclass_handler,
    enum_handler,
    callable_handler,
    float_range_handler,
    float_range_promote_handler,
    range_handler,
    coerced_handler,
    delegate_handler,
    object_method_old_new_handler,
    object_method_name_old_new_handler,
    member_method_object_old_new_handler
};


}  // namespace


PyObject*
Member::validate( CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    if( get_validate_mode() >= sizeof( handlers ) )
        return no_op_handler( this, atom, oldvalue, newvalue );  // LCOV_EXCL_LINE
    return handlers[ get_validate_mode() ]( this, atom, oldvalue, newvalue );
}


}  // namespace atom
