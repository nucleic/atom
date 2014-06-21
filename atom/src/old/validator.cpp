/*-----------------------------------------------------------------------------
| Copyright (c) 2013, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include <cppy/cppy.h>
#include <utils/py23compat.h>
#include "validator.h"


namespace atom
{

namespace
{

// true on success, false and exception on failure
bool check_context( Validator::Mode mode, PyObject* context )
{
    switch( mode )
    {
   	case Validator::Bool:
   	case Validator::Int:
   	case Validator::Float:
   		if( context != Py_True && context != Py_False )
   		{
   			cppy::type_error( context, "bool" );
   			return false;
   		}
   		break;
    // xxx validate a valid isinstance context for tuple, list and dict?
    case Validator::Dict:
    {
        if( !PyTuple_Check( context ) || PyTuple_GET_SIZE( context ) != 2 )
        {
            cppy::type_error( context, "2-tuple" );
            return false;
        }
        break;
    }
    // XXX validate a valid isinstance context for Instance?
    case Validator::Typed:
        if( !PyType_Check( context ) )
        {
            cppy::type_error( context, "type" );
            return false;
        }
        break;
    // XXX validate a valid issubclass context?
    case Validator::Enum:
        if( !PySequence_Check( context ) )
        {
            cppy::type_error( context, "sequence" );
            return false;
        }
        break;
    case Validator::Range:
    {
        if( !PyTuple_Check( context ) || PyTuple_GET_SIZE( context ) != 3 )
        {
            cppy::type_error( context, "3-tuple" );
            return false;
        }
        break;
    }
    case Validator::Coerced:
    {
        if( !PyTuple_Check( context ) || PyTuple_GET_SIZE( context ) != 2 )
        {
            cppy::type_error( context, "2-tuple of (type, callable)" );
            return false;
        }
        // XXX validate type as valid for isinstance(..., type)
        PyObject* coercer = PyTuple_GET_ITEM( context, 1 );
        if( !PyCallable_Check( coercer ) )
        {
            cppy::type_error( context, "2-tuple of (type, callable)" );
            return false;
        }
        break;
    }
    default:
        break;
    }
    return true;
}


PyObject* raise_validation_error(
    Validator* self, PyObject* atom, PyObject* name, PyObject* value )
{
    cppy::ptr args( PyTuple_Pack( 4, self, atom, name, value ) );
    if( !args )
    {
        return 0;
    }
    cppy::ptr res( PyObject_Call( self->m_error_handler, args.get(), 0 ) );
    if( !res )
    {
        return 0;
    }
    return cppy::bad_internal_call( "error handler did not raise exception" );
}


#define RAISE_VALIDATION_ERROR \
    return raise_validation_error( self, atom, name, value )


PyObject* bool_handler(
	Validator* self, PyObject* atom, PyObject* name, PyObject* value )
{
    if( PyBool_Check( value ) )
    {
        return cppy::incref( value );
    }
	if( self->m_context == Py_True )
	{
		long ok = PyObject_IsTrue( value );
		if( ok < 0 )
		{
			return 0;
		}
		return PyBool_FromLong( ok );
	}
    RAISE_VALIDATION_ERROR;
}


PyObject* int_handler(
	Validator* self, PyObject* atom, PyObject* name, PyObject* value )
{
    if( Py23Int_Check( value ) )
    {
        return cppy::incref( value );
    }
    if( self->m_context == Py_True )
    {
    	return Py23Number_Int( value );
    }
    RAISE_VALIDATION_ERROR;
}


PyObject* float_handler(
	Validator* self, PyObject* atom, PyObject* name, PyObject* value )
{
    if( PyFloat_Check( value ) )
    {
        return cppy::incref( value );
    }
    if( self->m_context == Py_True )
    {
    	return PyNumber_Float( value );
    }
    RAISE_VALIDATION_ERROR;
}


PyObject* bytes_handler(
	Validator* self, PyObject* atom, PyObject* name, PyObject* value )
{
    if( Py23Bytes_Check( value ) )
    {
        return cppy::incref( value );
    }
    RAISE_VALIDATION_ERROR;
}


PyObject* str_handler(
	Validator* self, PyObject* atom, PyObject* name, PyObject* value )
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
    RAISE_VALIDATION_ERROR;
}


PyObject* unicode_handler(
	Validator* self, PyObject* atom, PyObject* name, PyObject* value )
{
    if( PyUnicode_Check( value ) )
    {
        return cppy::incref( value );
    }
    RAISE_VALIDATION_ERROR;
}


PyObject* tuple_handler(
	Validator* self, PyObject* atom, PyObject* name, PyObject* value )
{
    if( !PyTuple_Check( value ) )
    {
        RAISE_VALIDATION_ERROR;
    }
   	PyObject* type = self->m_context;
    if( type == Py_None )
    {
        return cppy::incref( value );
    }
    Py_ssize_t size = PyTuple_GET_SIZE( value );
    cppy::ptr result( PyTuple_New( size ) );
    if( !result )
    {
        return 0;
    }
    for( Py_ssize_t i = 0; i < size; ++i )
    {
        PyObject* item = PyTuple_GET_ITEM( value, i );
        int r = PyObject_IsInstance( item, type );
        if( r == 1 )
        {
        	PyTuple_SET_ITEM( result.get(), i, cppy::incref( item ) );
            continue;
        }
        if( r == -1 )
        {
        	return 0;
        }
        RAISE_VALIDATION_ERROR;
    }
    return result.release();
}


PyObject* list_handler(
	Validator* self, PyObject* atom, PyObject* name, PyObject* value )
{
	if( !PyList_Check( value ) )
	{
		RAISE_VALIDATION_ERROR;
	}
	return PyList_GetSlice( value, 0, PyList_GET_SIZE( value ) );
	// XXX handle list types
}


PyObject* dict_handler(
	Validator* self, PyObject* atom, PyObject* name, PyObject* value )
{
    if( !PyDict_Check( value ) )
    {
        RAISE_VALIDATION_ERROR;
    }
    return PyDict_Copy( value );
    // XXX handle dict types
}


PyObject* instance_handler(
	Validator* self, PyObject* atom, PyObject* name, PyObject* value )
{
    if( value == Py_None )
    {
        return cppy::incref( value );
    }
    int r = PyObject_IsInstance( value, self->m_context );
    if( r == 1 )
    {
        return cppy::incref( value );
    }
    if( r == -1 )
    {
    	return 0;
    }
    RAISE_VALIDATION_ERROR;
}


PyObject* typed_handler(
	Validator* self, PyObject* atom, PyObject* name, PyObject* value )
{
    if( value == Py_None )
    {
        return cppy::incref( value );
    }
    PyTypeObject* type = reinterpret_cast<PyTypeObject*>( self->m_context );
    if( PyObject_TypeCheck( value, type ) )
    {
        return cppy::incref( value );
    }
    RAISE_VALIDATION_ERROR;
}


PyObject* subclass_handler(
	Validator* self, PyObject* atom, PyObject* name, PyObject* value )
{
	if( value == Py_None )
	{
		return cppy::incref( value );
	}
	int r = PyObject_IsSubclass( value, self->m_context );
    if( r == 1 )
    {
    	return cppy::incref( value );
    }
    if( r == -1 )
    {
    	return 0;
    }
    RAISE_VALIDATION_ERROR;
}


PyObject* enum_handler(
	Validator* self, PyObject* atom, PyObject* name, PyObject* value )
{
	int r = PySequence_Contains( self->m_context, value );
	if( r == 1 )
    {
        return cppy::incref( value );
    }
    if( r == -1 )
    {
    	return 0;
    }
    RAISE_VALIDATION_ERROR;
}


PyObject* callable_handler(
	Validator* self, PyObject* atom, PyObject* name, PyObject* value )
{
    if( value == Py_None || PyCallable_Check( value ) )
    {
        return cppy::incref( value );
    }
    RAISE_VALIDATION_ERROR;
}


PyObject* range_handler(
	Validator* self, PyObject* atom, PyObject* name, PyObject* value )
{
    PyObject* min_val = PyTuple_GET_ITEM( self->m_context, 0 );
    PyObject* max_val = PyTuple_GET_ITEM( self->m_context, 1 );
    PyObject* kind = PyTuple_GET_ITEM( self->m_context, 2 );
    int ok = PyObject_IsInstance( value, kind );
    if( ok == -1 )
    {
        return 0;
    }
    if( ok == 0 )
    {
        RAISE_VALIDATION_ERROR;
    }
    if( min_val != Py_None )
    {
        int r = PyObject_RichCompareBool( value, min_val, Py_LT );
        if( r == 1 )
        {
        	RAISE_VALIDATION_ERROR;
        }
        if( r == -1 )
        {
        	return 0;
        }
    }
    if( max_val != Py_None )
    {
        int r = PyObject_RichCompareBool( value, max_val, Py_GT );
        if( r == 1 )
        {
        	RAISE_VALIDATION_ERROR;
        }
        if( r == -1 )
        {
        	return 0;
        }
    }
    return cppy::incref( value );
}


PyObject* coerced_handler(
	Validator* self, PyObject* atom, PyObject* name, PyObject* value )
{
    PyObject* type = PyTuple_GET_ITEM( self->m_context, 0 );
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


Validator::Handler validate_handlers[] = {
	bool_handler,
    int_handler,
    float_handler,
    bytes_handler,
    str_handler,
    unicode_handler,
    tuple_handler,
    list_handler,
    dict_handler,
    instance_handler,
    typed_handler,
    subclass_handler,
    enum_handler,
    callable_handler,
    range_handler,
    coerced_handler,
};


PyObject* Validator_new( PyTypeObject* type, PyObject* args, PyObject* kwargs )
{
	if( kwargs )
	{
		return cppy::type_error( "__new__ takes no keyword arguments" );
	}
    long mode;
    PyObject* context;
    PyObject* error_handler;
    if( !PyArg_ParseTuple( args, "lOO", &mode, &context, &error_handler ) )
    {
        return 0;
    }
    if( mode < 0 || mode >= Validator::Last )
    {
    	return cppy::value_error( "validate mode out of range" );
    }
    if( !check_context( static_cast<Validator::Mode>( mode ), context ) )
    {
    	return 0;
    }
    if( !PyCallable_Check( error_handler ) )
    {
        return cppy::type_error( error_handler, "callable" );
    }
    PyObject* py_self = PyType_GenericNew( type, 0, 0 );
    if( !py_self )
    {
    	return 0;
    }
    Validator* self = reinterpret_cast<Validator*>( py_self );
    self->m_context = cppy::incref( context );
    self->m_error_handler = cppy::incref( error_handler );
    self->m_validate_handler = validate_handlers[ mode ];
    return py_self;
}


void Validator_clear( Validator* self )
{
    Py_CLEAR( self->m_context );
    Py_CLEAR( self->m_error_handler );
}


int Validator_traverse( Validator* self, visitproc visit, void* arg )
{
    Py_VISIT( self->m_context );
    Py_VISIT( self->m_error_handler );
    return 0;
}


void Validator_dealloc( Validator* self )
{
    PyObject_GC_UnTrack( self );
    Validator_clear( self );
    self->ob_type->tp_free( reinterpret_cast<PyObject*>( self ) );
}


// new ref on success, null and exception on failure
PyObject* Validator_call( Validator* self, PyObject* args, PyObject* kwargs )
{
	if( kwargs )
	{
		return cppy::type_error( "__call__ takes no keyword arguments" );
	}
	if( PyTuple_GET_SIZE( args ) != 3 )
	{
		PyErr_Format(
			PyExc_TypeError,
			"__call__ takes exactly 3 arguments (%zd given)",
			PyTuple_GET_SIZE( args ) );
		return 0;
	}
	PyObject* atom = PyTuple_GET_ITEM( args, 0 );
	PyObject* name = PyTuple_GET_ITEM( args, 1 );
	PyObject* value = PyTuple_GET_ITEM( args, 2 );
	return self->m_validate_handler( self, atom, name, value );
}


PyObject* Validator_get_context( Validator* self, void* ctxt )
{
    return cppy::incref( self->m_context );
}


PyObject* Validator_get_error_handler( Validator* self, void* ctxt )
{
    return cppy::incref( self->m_error_handler );
}


PyObject* Validator_get_mode( Validator* self, void* ctxt )
{
    for( long i = 0; i < Validator::Last; ++i )
    {
        if( validate_handlers[ i ] == self->m_validate_handler )
        {
            return Py23Int_FromLong( i );
        }
    }
    return cppy::bad_internal_call( "validate mode out of range" );
}


PyGetSetDef Validator_getset[] = {
    { "context",
      ( getter )Validator_get_context, ( setter )0,
      "Get the validation context for the validator." },
    { "error_handler",
      ( getter )Validator_get_error_handler, ( setter )0,
      "Get the error handler for the validator." },
    { "mode",
      ( getter )Validator_get_mode, ( setter )0,
      "Get the validation mode for the validator." },
    { 0 } // sentinel
};

} // namespace


PyTypeObject Validator::TypeObject = {
    PyObject_HEAD_INIT( &PyType_Type )
    0,                                  		/* ob_size */
    "atom.catom.CValidator",             		/* tp_name */
    sizeof( Validator ),                		/* tp_basicsize */
    0,                                  		/* tp_itemsize */
    ( destructor )Validator_dealloc,    		/* tp_dealloc */
    ( printfunc )0,                     		/* tp_print */
    ( getattrfunc )0,                   		/* tp_getattr */
    ( setattrfunc )0,                   		/* tp_setattr */
    ( cmpfunc )0,                       		/* tp_compare */
    ( reprfunc )0,                      		/* tp_repr */
    ( PyNumberMethods* )0,              		/* tp_as_number */
    ( PySequenceMethods* )0,            		/* tp_as_sequence */
    ( PyMappingMethods* )0,             		/* tp_as_mapping */
    ( hashfunc )0,                      		/* tp_hash */
    ( ternaryfunc )Validator_call,      		/* tp_call */
    ( reprfunc )0,                      		/* tp_str */
    ( getattrofunc )0,                  		/* tp_getattro */
    ( setattrofunc )0,                  		/* tp_setattro */
    ( PyBufferProcs* )0,                		/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC,	/* tp_flags */
    0,                                 			/* Documentation string */
    ( traverseproc )Validator_traverse,			/* tp_traverse */
    ( inquiry )Validator_clear,        			/* tp_clear */
    ( richcmpfunc )0,                  			/* tp_richcompare */
    0,                                 			/* tp_weaklistoffset */
    ( getiterfunc )0,                  			/* tp_iter */
    ( iternextfunc )0,                 			/* tp_iternext */
    ( struct PyMethodDef* )0,          			/* tp_methods */
    ( struct PyMemberDef* )0,          			/* tp_members */
    Validator_getset,			                /* tp_getset */
    0,                                 			/* tp_base */
    0,                                 			/* tp_dict */
    ( descrgetfunc )0,                 			/* tp_descr_get */
    ( descrsetfunc )0,                 			/* tp_descr_set */
    0,                                 			/* tp_dictoffset */
    ( initproc )0,           		   			/* tp_init */
    ( allocfunc )PyType_GenericAlloc,  			/* tp_alloc */
    ( newfunc )Validator_new,      				/* tp_new */
    ( freefunc )PyObject_GC_Del,       			/* tp_free */
    ( inquiry )0,                      			/* tp_is_gc */
    0,                                 			/* tp_bases */
    0,                                 			/* tp_mro */
    0,                                 			/* tp_cache */
    0,                                 			/* tp_subclasses */
    0,                                 			/* tp_weaklist */
    ( destructor )0                    			/* tp_del */
};


namespace
{

bool add_validate_mode( const char* name, Validator::Mode mode )
{
    cppy::ptr value( Py23Int_FromLong( mode ) );
    if( !value )
    {
        return false;
    }
    PyObject* type_dict = Validator::TypeObject.tp_dict;
    return PyDict_SetItemString( type_dict, name, value.get() ) == 0;
};

} // namespace


bool Validator::Ready()
{
	if( PyType_Ready( &TypeObject ) != 0 )
    {
        return false;
    }

#define STR_HELPER( x ) #x
#define STR( x ) STR_HELPER( x )
#define ADD_HELPER( a, b ) if( !add_validate_mode( a, b ) ) return false;
#define ADD_MODE( m ) ADD_HELPER( STR( m ), m )

    ADD_MODE( Bool )
    ADD_MODE( Int )
    ADD_MODE( Float )
    ADD_MODE( Bytes )
    ADD_MODE( Str )
    ADD_MODE( Unicode )
    ADD_MODE( Tuple )
    ADD_MODE( List )
    ADD_MODE( Dict )
    ADD_MODE( Instance )
    ADD_MODE( Typed )
    ADD_MODE( Subclass )
    ADD_MODE( Enum )
    ADD_MODE( Callable )
    ADD_MODE( Range )
    ADD_MODE( Coerced )

#undef ADD_MODE
#undef ADD_HELPER
#undef STR
#undef STR_HELPER

    return true;
}

} // namespace atom
