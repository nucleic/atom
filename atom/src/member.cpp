/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include "member.h"
#include "py23compat.h"

#include <cppy/cppy.h>


#define member_cast( o ) reinterpret_cast<Member*>( o )
#define pyobject_cast( o ) reinterpret_cast<PyObject*>( o )
#define pytype_cast( o ) reinterpret_cast<PyTypeObject*>( o )


namespace atom
{

PyObject* ValidationError;


namespace
{

PyObject* clone_str;
PyObject* empty_tuple;


PyObject* validation_error( Member* member, PyObject* atom, PyObject* name, PyObject* value )
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
	cppy::ptr method( PyObject_GetAttrString( pyobject_cast( member ), "validation_error" ) );
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
	cppy::system_error( "member failed to raise validation error" );
	return 0;
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
		{
			// XXX validate a valid isinstance context for Instance?
			break;
		}
		case Member::ValidateSubclass:
		{
			// XXX validate a valid issubclass context for Subclass?
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
	PyObject* ctxt = member->m_validate_context;
	if( PyObject_TypeCheck( value, pytype_cast( ctxt ) ) )
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


PyObject* validate_call_object( Member* member, PyObject* atom, PyObject* name, PyObject* value )
{
	cppy::ptr args( PyTuple_Pack( 3, atom, name, value ) );
	if( !args )
	{
		return 0;
	}
	return PyObject_Call( member->m_validate_context, args.get(), 0 );
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


typedef PyObject* ( *DefaultHandler )( Member* member, PyObject* atom, PyObject* name );


typedef PyObject* ( *ValidateHandler )(	Member* member, PyObject* atom, PyObject* name, PyObject* value );


DefaultHandler default_handlers[] = {
	default_noop,
	default_value,
	default_factory,
	default_call_object,
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
	validate_call_object,
	validate_member_method
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
	self->ob_type->tp_free( pyobject_cast( self ) );
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


PyObject* Member_get_value_index( Member* self, void* context )
{
	return Py23Int_FromLong( static_cast<long>( self->valueIndex() ) );
}


PyObject* Member_clone( Member* self, PyObject* args )
{
	cppy::ptr pyo( PyType_GenericNew( self->ob_type, 0, 0 ) );
	if( !pyo )
	{
		return 0;
	}
	Member* clone = member_cast( pyo.get() );
	if( self->m_metadata && !( clone->m_metadata = PyDict_Copy( self->m_metadata ) ) )
	{
		return 0;
	}
	clone->m_default_context = cppy::xincref( self->m_default_context );
	clone->m_validate_context = cppy::xincref( self->m_validate_context );
	clone->m_value_index = self->m_value_index;
	clone->m_default_mode = self->m_default_mode;
	clone->m_validate_mode = self->m_validate_mode;
	return pyo.release();
}


PyObject* Member_do_default( Member* self, PyObject* args )
{
	PyObject* atom;
	PyObject* name;
	if( !PyArg_ParseTuple( args, "OO", &atom, &name ) )
	{
		return 0;
	}
	return self->defaultValue( atom, name );
}


PyObject* Member_do_validate( Member* self, PyObject* args )
{
	PyObject* atom;
	PyObject* name;
	PyObject* value;
	if( !PyArg_ParseTuple( args, "OOO", &atom, &name, &value ) )
	{
		return 0;
	}
	return self->validateValue( atom, name, value );
}


PyGetSetDef Member_getset[] = {
	{ "metadata",
	  ( getter )Member_get_metadata,
	  ( setter )Member_set_metadata,
	  "metadata for the member (if defined)" },
	{ "default_mode",
	  ( getter )Member_get_default_mode,
	  ( setter )Member_set_default_mode,
	  "the default value mode for the member" },
	{ "validate_mode",
	  ( getter )Member_get_validate_mode,
	  ( setter )Member_set_validate_mode,
	  "the validate mode for the member" },
	{ "_value_index",
	  ( getter )Member_get_value_index, 0,
	  "*private* the read-only value index for the member" },
	{ 0 } // sentinel
};


PyMethodDef Member_methods[] = {
	{ "clone",
	  ( PyCFunction )Member_clone,
	  METH_NOARGS,
	  "Create a clone of the member." },
	{ "do_default",
	  ( PyCFunction )Member_do_default,
	  METH_VARARGS,
	  "Run the default value handler for the member." },
	{ "do_validate",
	  ( PyCFunction )Member_do_validate,
	  METH_VARARGS,
	  "Run the validate value handler for the member." },
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
	if( !( ValidationError = PyErr_NewException( "catom.ValidationError", 0, 0 ) ) )
	{
		return false;
	}
	if( !( clone_str = PyString_FromString( "clone" ) ) )
	{
		return false;
	}
	if( !( empty_tuple = PyTuple_New( 0 ) ) )
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
	ADD_MODE( ValidateCallObject )
	ADD_MODE( ValidateMemberMethod )

#undef ADD_MODE
#undef ADD_HELPER
#undef STR
#undef STR_HELPER

	return true;
}


PyObject* Member::Clone( PyObject* member )
{
	cppy::ptr method( PyObject_GetAttr( member, clone_str ) );
	if( !method )
	{
		return 0;
	}
	cppy::ptr clone( method.call( empty_tuple ) );
	if( !clone )
	{
		return 0;
	}
	if( !Member::TypeCheck( clone.get() ) )
	{
		return cppy::type_error( "member.clone() returned a non-Member type" );
	}
	return clone.release();
}


PyObject* Member::defaultValue( PyObject* atom, PyObject* name )
{
	cppy::ptr value( default_handlers[ m_default_mode ]( this, atom, name ) );
	return value ? validateValue( atom, name, value.get() ) : 0;
}


PyObject* Member::validateValue( PyObject* atom, PyObject* name, PyObject* value )
{
	return validate_handlers[ m_validate_mode ]( this, atom, name, value );
}

} // namespace atom
