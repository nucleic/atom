/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic
|
| Distributed under the terms of the BSD 3-Clause License.
|
| The full license is in the file LICENSE, distributed with this software.
|----------------------------------------------------------------------------*/
#include "member.h"
#include "atom.h"
#include "errors.h"
#include "utils.h"
#include "py23compat.h"

#include <cppy/cppy.h>


#define member_cast( o ) reinterpret_cast<Member*>( o )
#define pyobject_cast( o ) reinterpret_cast<PyObject*>( o )
#define pytype_cast( o ) reinterpret_cast<PyTypeObject*>( o )


namespace atom
{

namespace
{

PyObject* validation_error( Member* member, PyObject* atom, PyObject* name, PyObject* value )
{
	cppy::ptr memberptr( pyobject_cast( member ), true );
	cppy::ptr method( memberptr.getattr( "validation_error" ) );
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
	return cppy::system_error( "member failed to raise validation error" );
}


bool check_context( Member::DefaultMode mode, PyObject* context )
{
	switch( mode )
	{
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
		case Member::ValidateTyped:
		{
			if( !PyType_Check( context ) )
			{
				cppy::type_error( context, "type" );
				return false;
			}
			break;
		}
		case Member::ValidateInstance:
		case Member::ValidateSubclass:
		{
			if( !utils::is_type_or_tuple_of_types( context ) )
			{
				cppy::type_error( context, "type or tuple of types" );
				return false;
			}
			break;
		}
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
			PyObject* kind = PyTuple_GET_ITEM( context, 2 );
			if( !utils::is_type_or_tuple_of_types( kind ) )
			{
				cppy::type_error( kind, "type or tuple of types" );
				return false;
			}
			break;
		}
		case Member::ValidateCoerced:
		case Member::ValidateList:
		case Member::ValidateSet:
		{
			if( !PyTuple_Check( context ) || PyTuple_GET_SIZE( context ) != 2 )
			{
				cppy::type_error( context, "2-tuple of (kind, callable)" );
				return false;
			}
			PyObject* kind = PyTuple_GET_ITEM( context, 0 );
			if( !utils::is_type_or_tuple_of_types( kind ) )
			{
				cppy::type_error( kind, "type or tuple of types" );
				return false;
			}
			PyObject* callable = PyTuple_GET_ITEM( context, 1 );
			if( !PyCallable_Check( callable ) )
			{
				cppy::type_error( callable, "callable" );
				return false;
			}
			break;
		}
		case Member::ValidateTuple:
		{
			if( !utils::is_type_or_tuple_of_types( context ) )
			{
				cppy::type_error( context, "type or tuple of types" );
				return false;
			}
			break;
		}
		case Member::ValidateDict:
		{
			if( !PyTuple_Check( context ) || PyTuple_GET_SIZE( context ) != 3 )
			{
				cppy::type_error( context, "3-tuple of (kind, kind, callable)" );
				return false;
			}
			PyObject* key_type = PyTuple_GET_ITEM( context, 0 );
			if( !utils::is_type_or_tuple_of_types( key_type ) )
			{
				cppy::type_error( key_type, "type or tuple of types" );
				return false;
			}
			PyObject* value_type = PyTuple_GET_ITEM( context, 1 );
			if( !utils::is_type_or_tuple_of_types( value_type ) )
			{
				cppy::type_error( value_type, "type or tuple of types" );
				return false;
			}
			PyObject* callable = PyTuple_GET_ITEM( context, 2 );
			if( !PyCallable_Check( callable ) )
			{
				cppy::type_error( callable, "callable" );
				return false;
			}
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


PyObject* default_noop( Member* member, PyObject* atom, PyObject* name )
{
	return cppy::incref( Py_None );
}


PyObject* default_value( Member* member, PyObject* atom, PyObject* name )
{
	return cppy::incref( member->m_default_context );
}


PyObject* default_factory( Member* member, PyObject* atom, PyObject* name )
{
	cppy::ptr args( PyTuple_New( 0 ) );
	if( !args )
	{
		return 0;
	}
	return PyObject_Call( member->m_default_context, args.get(), 0 );
}


PyObject* default_call_object( Member* member, PyObject* atom, PyObject* name )
{
	cppy::ptr args( PyTuple_Pack( 2, atom, name ) );
	if( !args )
	{
		return 0;
	}
	return PyObject_Call( member->m_default_context, args.get(), 0 );
}


PyObject* default_atom_method( Member* member, PyObject* atom, PyObject* name )
{
	cppy::ptr method( PyObject_GetAttr( pyobject_cast( atom ), member->m_default_context ) );
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


PyObject* default_member_method( Member* member, PyObject* atom, PyObject* name )
{
	cppy::ptr method( PyObject_GetAttr( pyobject_cast( member ), member->m_default_context ) );
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


PyObject* validate_noop( Member* member, PyObject* atom, PyObject* name, PyObject* value )
{
	return cppy::incref( value );
}


PyObject* validate_bool( Member* member, PyObject* atom, PyObject* name, PyObject* value )
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


PyObject* validate_int( Member* member, PyObject* atom, PyObject* name, PyObject* value )
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


PyObject* validate_float( Member* member, PyObject* atom, PyObject* name, PyObject* value )
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


PyObject* validate_bytes( Member* member, PyObject* atom, PyObject* name, PyObject* value )
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


PyObject* validate_str( Member* member, PyObject* atom, PyObject* name, PyObject* value )
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


PyObject* validate_unicode( Member* member, PyObject* atom, PyObject* name, PyObject* value )
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


PyObject* validate_typed( Member* member, PyObject* atom, PyObject* name, PyObject* value )
{
	if( value == Py_None )
	{
		return cppy::incref( value );
	}
	if( PyObject_TypeCheck( value, pytype_cast( member->m_validate_context ) ) )
	{
		return cppy::incref( value );
	}
	return validation_error( member, atom, name, value );
}


PyObject* validate_instance( Member* member, PyObject* atom, PyObject* name, PyObject* value )
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


PyObject* validate_subclass( Member* member, PyObject* atom, PyObject* name, PyObject* value )
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


PyObject* validate_enum( Member* member, PyObject* atom, PyObject* name, PyObject* value )
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


PyObject* validate_callable( Member* member, PyObject* atom, PyObject* name, PyObject* value )
{
	if( value == Py_None || PyCallable_Check( value ) )
	{
		return cppy::incref( value );
	}
	return validation_error( member, atom, name, value );
}


PyObject* validate_range( Member* member, PyObject* atom, PyObject* name, PyObject* value )
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


PyObject* validate_coerced( Member* member, PyObject* atom, PyObject* name, PyObject* value )
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
		if( PyErr_ExceptionMatches( PyExc_TypeError ) || PyErr_ExceptionMatches( PyExc_ValueError ) )
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


PyObject* ttv_error( PyObject* value_type, PyObject* value )
{
	static PyObject* ttv_message = 0;
	if( !ttv_message )
	{
		cppy::ptr mod( PyImport_ImportModule( "atom._cpphelpers" ) );
		if( !mod )
		{
			return 0;
		}
		ttv_message = mod.getattr( "typed_tuple_validation_message" );
		if( !ttv_message )
		{
			return 0;
		}
	}
	cppy::ptr args( PyTuple_Pack( 2, value_type, value ) );
	if( !args )
	{
		return 0;
	}
	cppy::ptr msg( PyObject_Call( ttv_message, args.get(), 0 ) );
	if( !msg )
	{
		return 0;
	}
	PyErr_SetObject( Errors::ValidationError, msg.get() );
	return 0;
};


PyObject* validate_tuple( Member* member, PyObject* atom, PyObject* name, PyObject* value )
{
	if( !PyTuple_Check( value ) )
	{
		return validation_error( member, atom, name, value );
	}
	PyObject* value_type = member->m_validate_context;
	if( value_type == pyobject_cast( &PyBaseObject_Type ) )
	{
		return cppy::incref( value );
	}
	Py_ssize_t count = PyTuple_GET_SIZE( value );
	for( Py_ssize_t i = 0; i < count; ++i )
	{
		int ok = PyObject_IsInstance( PyTuple_GET_ITEM( value, i ), value_type );
		if( ok == 0 )
		{
			return ttv_error( value_type, PyTuple_GET_ITEM( value, i ) );
		}
		if( ok == -1 )
		{
			return 0;
		}
	}
	return cppy::incref( value );
}


PyObject* validate_list( Member* member, PyObject* atom, PyObject* name, PyObject* value )
{
	if( !PyList_Check( value ) )
	{
		return validation_error( member, atom, name, value );
	}
	PyObject* kind = PyTuple_GET_ITEM( member->m_validate_context, 0 );
	PyObject* callable = PyTuple_GET_ITEM( member->m_validate_context, 1 );
	cppy::ptr args( PyTuple_Pack( 2, kind, value ) );
	cppy::ptr result( PyObject_Call( callable, args.get(), 0 ) );
	if( !result )
	{
		return 0;
	}
	if( !PyList_Check( result.get() ) )
	{
		return validation_error( member, atom, name, result.get() );
	}
	return result.release();
}


PyObject* validate_dict( Member* member, PyObject* atom, PyObject* name, PyObject* value )
{
	if( !PyDict_Check( value ) )
	{
		return validation_error( member, atom, name, value );
	}
	PyObject* key_type = PyTuple_GET_ITEM( member->m_validate_context, 0 );
	PyObject* value_type = PyTuple_GET_ITEM( member->m_validate_context, 1 );
	PyObject* callable = PyTuple_GET_ITEM( member->m_validate_context, 2 );
	cppy::ptr args( PyTuple_Pack( 3, key_type, value_type, value ) );
	cppy::ptr result( PyObject_Call( callable, args.get(), 0 ) );
	if( !result )
	{
		return 0;
	}
	if( !PyDict_Check( result.get() ) )
	{
		return validation_error( member, atom, name, result.get() );
	}
	return result.release();
}


PyObject* validate_set( Member* member, PyObject* atom, PyObject* name, PyObject* value )
{
	if( !PyAnySet_Check( value ) )
	{
		return validation_error( member, atom, name, value );
	}
	PyObject* kind = PyTuple_GET_ITEM( member->m_validate_context, 0 );
	PyObject* callable = PyTuple_GET_ITEM( member->m_validate_context, 1 );
	cppy::ptr args( PyTuple_Pack( 2, kind, value ) );
	cppy::ptr result( PyObject_Call( callable, args.get(), 0 ) );
	if( !result )
	{
		return 0;
	}
	if( !PySet_Check( result.get() ) )
	{
		return validation_error( member, atom, name, result.get() );
	}
	return result.release();
}


PyObject* validate_call_object( Member* member, PyObject* atom, PyObject* name, PyObject* value )
{
	cppy::ptr args( PyTuple_Pack( 3, atom, name, value ) );
	if( !args )
	{
		return 0;
	}
	return PyObject_Call( member->m_validate_context, args.get(), 0 );
}


PyObject* validate_atom_method( Member* member, PyObject* atom, PyObject* name, PyObject* value )
{
	cppy::ptr method( PyObject_GetAttr( pyobject_cast( atom ), member->m_validate_context ) );
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


PyObject* validate_member_method( Member* member, PyObject* atom, PyObject* name, PyObject* value )
{
	cppy::ptr method( PyObject_GetAttr( pyobject_cast( member ), member->m_validate_context ) );
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


PyObject* post_validate_noop( Member* member, PyObject* atom, PyObject* name, PyObject* value )
{
	return cppy::incref( value );
}


PyObject* post_validate_call_object( Member* member, PyObject* atom, PyObject* name, PyObject* value )
{
	cppy::ptr args( PyTuple_Pack( 3, atom, name, value ) );
	if( !args )
	{
		return 0;
	}
	return PyObject_Call( member->m_post_validate_context, args.get(), 0 );
}


PyObject* post_validate_atom_method( Member* member, PyObject* atom, PyObject* name, PyObject* value )
{
	cppy::ptr method( PyObject_GetAttr( pyobject_cast( atom ), member->m_post_validate_context ) );
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


PyObject* post_validate_member_method( Member* member, PyObject* atom, PyObject* name, PyObject* value )
{
	cppy::ptr method( PyObject_GetAttr( pyobject_cast( member ), member->m_post_validate_context ) );
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


typedef PyObject* ( *DefaultHandler )( Member* member, PyObject* atom, PyObject* name );


typedef PyObject* ( *ValidateHandler )(	Member* member, PyObject* atom, PyObject* name, PyObject* value );


typedef PyObject* ( *PostValidateHandler )(	Member* member, PyObject* atom, PyObject* name, PyObject* value );


DefaultHandler default_handlers[] = {
	default_noop,
	default_value,
	default_factory,
	default_call_object,
	default_atom_method,
	default_member_method
};


ValidateHandler validate_handlers[] = {
	validate_noop,
	validate_bool,
	validate_int,
	validate_float,
	validate_bytes,
	validate_str,
	validate_unicode,
	validate_typed,
	validate_instance,
	validate_subclass,
	validate_enum,
	validate_callable,
	validate_range,
	validate_coerced,
	validate_tuple,
	validate_list,
	validate_dict,
	validate_set,
	validate_call_object,
	validate_atom_method,
	validate_member_method
};


PostValidateHandler post_validate_handlers[] = {
	post_validate_noop,
	post_validate_call_object,
	post_validate_atom_method,
	post_validate_member_method
};


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


template <typename MODE>
struct ParseMode
{
	int operator()( PyObject* arg, uint8_t* mode, PyObject** context )
	{
		if( !PyTuple_Check( arg ) || PyTuple_GET_SIZE( arg ) != 2 )
		{
			cppy::type_error( arg, "2-tuple" );
			return -1;
		}
		long val;
		PyObject* ctxt;
		if( !PyArg_ParseTuple( arg, "lO", &val, &ctxt ) )
		{
			return -1;
		}
		if( val < 0 || val >= ModeTraits<MODE>::Last )
		{
			cppy::value_error( "mode out of range" );
			return -1;
		}
		if( !check_context( static_cast<MODE>( val ), ctxt ) )
		{
			return -1;
		}
		*mode = static_cast<uint8_t>( val );
		cppy::replace( context, ctxt );
		return 0;
	}
};


PyObject* packMode( uint8_t mode, PyObject* context )
{
	PyObject* pymode = Py23Int_FromLong( static_cast<long>( mode ) );
	if( !pymode )
	{
		return 0;
	}
	return PyTuple_Pack( 2, pymode, context ? context : Py_None );
}


int Member_init( PyObject* self, PyObject* args, PyObject* kwargs )
{
	if( PyTuple_GET_SIZE( args ) > 0 )
	{
		cppy::type_error( "__init__() takes no positional arguments" );
		return -1;
	}
	if( kwargs && PyDict_Size( kwargs ) )
	{
		cppy::ptr copy( PyDict_Copy( kwargs ) );
		if( !copy )
		{
			return -1;
		}
		Member* member = member_cast( self );
		cppy::replace( &member->m_metadata , copy.get() );
	}
	return 0;
}


int Member_clear( Member* self )
{
	Py_CLEAR( self->m_metadata );
	Py_CLEAR( self->m_default_context );
	Py_CLEAR( self->m_validate_context );
	return 0;
}


int Member_traverse( Member* self, visitproc visit, void* arg )
{
	Py_VISIT( self->m_metadata );
	Py_VISIT( self->m_default_context );
	Py_VISIT( self->m_validate_context );
	return 0;
}


void Member_dealloc( Member* self )
{
	PyObject_GC_UnTrack( self );
	Member_clear( self );
	Py_TYPE( self )->tp_free( pyobject_cast( self ) );
}


PyObject* Member_get_index( Member* self, void* context )
{
	return Py23Int_FromSsize_t( self->m_index );
}


int Member_set_index( Member* self, PyObject* value, void* context )
{
	if( !value )
	{
		cppy::attribute_error( "can't delete attribute" );
		return -1;
	}
	if( !Py23Int_Check( value ) )
	{
		cppy::type_error( value, "int" );
		return -1;
	}
	Py_ssize_t index = Py23Int_AsSsize_t( value );
	if( index < 0 )
	{
		cppy::system_error( "invalid member index" );
		return -1;
	}
	self->m_index = index;
	return 0;
}


PyObject* Member_get_metadata( Member* self, void* context )
{
	return cppy::incref( self->m_metadata ? self->m_metadata : Py_None );
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


PyObject* Member_get_default_mode( Member* self, void* context )
{
	return packMode( self->m_default_mode, self->m_default_context );
}


int Member_set_default_mode( Member* self, PyObject* arg )
{
	return ParseMode<Member::DefaultMode>()( arg, &self->m_default_mode, &self->m_default_context );
}


PyObject* Member_get_validate_mode( Member* self, void* context )
{
	return packMode( self->m_validate_mode, self->m_validate_context );
}


int Member_set_validate_mode( Member* self, PyObject* arg )
{
	return ParseMode<Member::ValidateMode>()( arg, &self->m_validate_mode, &self->m_validate_context );
}


PyObject* Member_get_post_validate_mode( Member* self, void* context )
{
	return packMode( self->m_post_validate_mode, self->m_post_validate_context );
}


int Member_set_post_validate_mode( Member* self, PyObject* arg )
{
	return ParseMode<Member::PostValidateMode>()( arg, &self->m_post_validate_mode, &self->m_post_validate_context );
}


PyObject* Member_clone( Member* self, PyObject* args, PyObject* kwargs )
{
	cppy::ptr pyo( Py_TYPE( self )->tp_new( Py_TYPE( self ), args, kwargs ) );
	if( !pyo )
	{
		return 0;
	}
	cppy::ptr metadata;
	if( self->m_metadata && !( metadata = PyDict_Copy( self->m_metadata ) ) )
	{
		return 0;
	}
	Member* clone = member_cast( pyo.get() );
	cppy::replace( &self->m_metadata, metadata.get() );
	cppy::replace( &clone->m_default_context, self->m_default_context );
	cppy::replace( &clone->m_validate_context, self->m_validate_context );
	cppy::replace( &clone->m_post_validate_context, self->m_post_validate_context );
	clone->m_index = self->m_index;
	clone->m_default_mode = self->m_default_mode;
	clone->m_validate_mode = self->m_validate_mode;
	clone->m_post_validate_mode = self->m_post_validate_mode;
	return pyo.release();
}


PyObject* Member_do_default_value( Member* self, PyObject* args )
{
	PyObject* atom;
	PyObject* name;
	if( !PyArg_UnpackTuple( args, "do_default_value", 2, 2, &atom, &name ) )
	{
		return 0;
	}
	if( !Atom::TypeCheck( atom ) )
	{
		return cppy::type_error( atom, "Atom" );
	}
	if( !Py23Str_Check( name ) )
	{
		return cppy::type_error( name, "str" );
	}
	return self->defaultValue( atom, name );
}


PyObject* Member_do_validate( Member* self, PyObject* args )
{
	PyObject* atom;
	PyObject* name;
	PyObject* value;
	if( !PyArg_UnpackTuple( args, "do_validate", 3, 3, &atom, &name, &value ) )
	{
		return 0;
	}
	if( !Atom::TypeCheck( atom ) )
	{
		return cppy::type_error( atom, "Atom" );
	}
	if( !Py23Str_Check( name ) )
	{
		return cppy::type_error( name, "str" );
	}
	return self->validate( atom, name, value );
}


PyObject* Member_do_post_validate( Member* self, PyObject* args )
{
	PyObject* atom;
	PyObject* name;
	PyObject* value;
	if( !PyArg_UnpackTuple( args, "do_post_validate", 3, 3, &atom, &name, &value ) )
	{
		return 0;
	}
	if( !Atom::TypeCheck( atom ) )
	{
		return cppy::type_error( atom, "Atom" );
	}
	if( !Py23Str_Check( name ) )
	{
		return cppy::type_error( name, "str" );
	}
	return self->postValidate( atom, name, value );
}


PyObject* Member_validation_error( Member* self, PyObject* args )
{
	static PyObject* mv_message = 0;
	if( !mv_message )
	{
		cppy::ptr mod( PyImport_ImportModule( "atom._cpphelpers" ) );
		if( !mod )
		{
			return 0;
		}
		mv_message = mod.getattr( "member_validation_message" );
		if( !mv_message )
		{
			return 0;
		}
	}
	cppy::ptr callargs( PyTuple_Pack( 2, pyobject_cast( self ), args ) );
	if( !callargs )
	{
		return 0;
	}
	cppy::ptr msg( PyObject_Call( mv_message, callargs.get(), 0 ) );
	if( !msg )
	{
		return 0;
	}
	PyErr_SetObject( Errors::ValidationError, msg.get() );
	return 0;
};


PyGetSetDef Member_getset[] = {
	{ "index",
		( getter )Member_get_index,
		( setter )Member_set_index,
		"the memory index for the member", 0 },
	{ "metadata",
		( getter )Member_get_metadata,
		( setter )Member_set_metadata,
		"metadata for the member (if defined)", 0 },
	{ "default_mode",
		( getter )Member_get_default_mode,
		( setter )Member_set_default_mode,
		"the default value mode for the member", 0 },
	{ "validate_mode",
		( getter )Member_get_validate_mode,
		( setter )Member_set_validate_mode,
		"the validate mode for the member", 0 },
	{ "post_validate_mode",
		( getter )Member_get_post_validate_mode,
		( setter )Member_set_post_validate_mode,
		"the post validate mode for the member", 0 },
	{ 0 } // sentinel
};


PyMethodDef Member_methods[] = {
	{ "clone",
		( PyCFunction )Member_clone,
		METH_VARARGS | METH_KEYWORDS,
		"Create a clone of the member." },
	{ "do_default_value",
		( PyCFunction )Member_do_default_value,
		METH_VARARGS,
		"Run the default value handler for the member." },
	{ "do_validate",
		( PyCFunction )Member_do_validate,
		METH_VARARGS,
		"Run the validate value handler for the member." },
	{ "do_post_validate",
		( PyCFunction )Member_do_post_validate,
		METH_VARARGS,
		"Run the post validate value handler for the member." },
	{ "validation_error",
		( PyCFunction )Member_validation_error,
		METH_VARARGS,
		"Raise a ValidationError for the member." },
	{ 0 } // sentinel
};


template <typename MODE>
bool add_member_mode( const char* name, MODE mode )
{
	cppy::ptr value( Py23Int_FromLong( static_cast<long>( mode ) ) );
	if( !value )
	{
		return false;
	}
	PyObject* type_dict = Member::TypeObject.tp_dict;
	return PyDict_SetItemString( type_dict, name, value.get() ) == 0;
}

} // namespace


PyTypeObject Member::TypeObject = {
	PyVarObject_HEAD_INIT( &PyType_Type, 0 )
	"atom.catom.CMember",                  /* tp_name */
	sizeof( Member ),                      /* tp_basicsize */
	0,                                     /* tp_itemsize */
	( destructor )Member_dealloc,          /* tp_dealloc */
	( printfunc )0,                        /* tp_print */
	( getattrfunc )0,                      /* tp_getattr */
	( setattrfunc )0,                      /* tp_setattr */
#ifdef IS_PY3K
	( void* )0,                            /* tp_reserved */
#else
	( cmpfunc )0,                          /* tp_compare */
#endif
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
	( initproc )Member_init,               /* tp_init */
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

#define STR_HELPER( x ) #x
#define STR( x ) STR_HELPER( x )
#define ADD_HELPER( a, b ) if( !add_member_mode( a, b ) ) return false;
#define ADD_MODE( m ) ADD_HELPER( STR( m ), m )

	ADD_MODE( NoDefault )
	ADD_MODE( DefaultValue )
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
	ADD_MODE( ValidateTyped )
	ADD_MODE( ValidateInstance )
	ADD_MODE( ValidateSubclass )
	ADD_MODE( ValidateEnum )
	ADD_MODE( ValidateCallable )
	ADD_MODE( ValidateRange )
	ADD_MODE( ValidateCoerced )
	ADD_MODE( ValidateTuple )
	ADD_MODE( ValidateList )
	ADD_MODE( ValidateDict )
	ADD_MODE( ValidateSet )
	ADD_MODE( ValidateCallObject )
	ADD_MODE( ValidateAtomMethod )
	ADD_MODE( ValidateMemberMethod )

	ADD_MODE( NoPostValidate )
	ADD_MODE( PostValidateCallObject )
	ADD_MODE( PostValidateAtomMethod )
	ADD_MODE( PostValidateMemberMethod )

#undef ADD_MODE
#undef ADD_HELPER
#undef STR
#undef STR_HELPER

	return true;
}


PyObject* Member::defaultValue( PyObject* atom, PyObject* name )
{
	return default_handlers[ m_default_mode ]( this, atom, name );
}


PyObject* Member::validate( PyObject* atom, PyObject* name, PyObject* value )
{
	return validate_handlers[ m_validate_mode ]( this, atom, name, value );
}


PyObject* Member::postValidate( PyObject* atom, PyObject* name, PyObject* value )
{
	return post_validate_handlers[ m_post_validate_mode ]( this, atom, name, value );
}

} // namespace atom
