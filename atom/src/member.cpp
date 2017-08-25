/*-----------------------------------------------------------------------------
| Copyright (c) 2013, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#ifdef __clang__
#pragma clang diagnostic ignored "-Wdeprecated-writable-strings"
#endif

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif

#include "member.h"
#include "enumtypes.h"
#include "packagenaming.h"
#include "py23compat.h"

using namespace PythonHelpers;


static PyObject* undefined;


static PyObject*
Member_new( PyTypeObject* type, PyObject* args, PyObject* kwargs )
{
    PyObjectPtr selfptr( PyType_GenericNew( type, args, kwargs ) );
    if( !selfptr )
        return 0;
    Member* member = member_cast( selfptr.get() );
    member->name = newref( undefined );
    member->set_getattr_mode( GetAttr::Slot );
    member->set_setattr_mode( SetAttr::Slot );
    member->set_delattr_mode( DelAttr::Slot );
    return selfptr.release();
}


static void
Member_clear( Member* self )
{
    Py_CLEAR( self->name );
    Py_CLEAR( self->metadata );
    Py_CLEAR( self->getattr_context );
    Py_CLEAR( self->setattr_context );
    Py_CLEAR( self->delattr_context );
    Py_CLEAR( self->validate_context );
    Py_CLEAR( self->post_getattr_context );
    Py_CLEAR( self->post_setattr_context );
    Py_CLEAR( self->default_value_context );
    Py_CLEAR( self->post_validate_context );
    if( self->static_observers )
        self->static_observers->clear();
}


static int
Member_traverse( Member* self, visitproc visit, void* arg )
{
    Py_VISIT( self->name );
    Py_VISIT( self->metadata );
    Py_VISIT( self->getattr_context );
    Py_VISIT( self->setattr_context );
    Py_VISIT( self->delattr_context );
    Py_VISIT( self->validate_context );
    Py_VISIT( self->post_getattr_context );
    Py_VISIT( self->post_setattr_context );
    Py_VISIT( self->default_value_context );
    Py_VISIT( self->post_validate_context );
    if( self->static_observers )
    {
        std::vector<PyObjectPtr>::iterator it;
        std::vector<PyObjectPtr>::iterator end = self->static_observers->end();
        for( it = self->static_observers->begin(); it != end; ++it )
            Py_VISIT( it->get() );
    }
    return 0;
}


static void
Member_dealloc( Member* self )
{
    PyObject_GC_UnTrack( self );
    Member_clear( self );
    delete self->static_observers;
    self->static_observers = 0;
    Py_TYPE(self)->tp_free( pyobject_cast( self ) );
}


static PyObject*
Member_has_observers( Member* self )
{
    return py_bool( self->has_observers() );
}


static PyObject*
Member_has_observer( Member* self, PyObject* observer )
{
    if( !Py23Str_CheckExact( observer ) && !PyCallable_Check( observer ) )
        return py_expected_type_fail( observer, "str or callable" );
    return py_bool( self->has_observer( observer ) );
}


static PyObject*
Member_copy_static_observers( Member* self, PyObject* other )
{
    if( !Member::TypeCheck( other ) )
        return py_expected_type_fail( other, "Member" );
    Member* member = member_cast( other );
    if( self == member )
        Py_RETURN_NONE;
    if( !member->static_observers )
    {
        delete self->static_observers;
        self->static_observers = 0;
    }
    else
    {
        if( !self->static_observers )
            self->static_observers = new std::vector<PyObjectPtr>();
        *self->static_observers = *member->static_observers;
    }
    Py_RETURN_NONE;
}


static PyObject*
Member_static_observers( Member* self )
{
    if( !self->static_observers )
        return PyTuple_New( 0 );
    std::vector<PyObjectPtr>& observers( *self->static_observers );
    size_t size = observers.size();
    PyObject* items = PyTuple_New( size );
    if( !items )
        return 0;
    for( size_t i = 0; i < size; ++i )
        PyTuple_SET_ITEM( items, i, observers[ i ].newref() );
    return items;
}


static PyObject*
Member_add_static_observer( Member* self, PyObject* observer )
{
    if( !Py23Str_CheckExact( observer ) && !PyCallable_Check( observer ) )
        return py_expected_type_fail( observer, "str or callable" );
    self->add_observer( observer );
    Py_RETURN_NONE;
}


static PyObject*
Member_remove_static_observer( Member* self, PyObject* observer )
{
    if( !Py23Str_CheckExact( observer ) && !PyCallable_Check( observer ) )
        return py_expected_type_fail( observer, "str or callable" );
    self->remove_observer( observer );
    Py_RETURN_NONE;
}


static PyObject*
Member_get_slot( Member* self, PyObject* object )
{
    if( !CAtom::TypeCheck( object ) )
        return py_expected_type_fail( object, "CAtom" );
    CAtom* atom = catom_cast( object );
    if( self->index >= atom->get_slot_count() )
        return py_no_attr_fail( object, (char *)Py23Str_AS_STRING( self->name ) );
    PyObjectPtr value( atom->get_slot( self->index ) );
    if( value )
        return value.release();
    Py_RETURN_NONE;
}


static PyObject*
Member_set_slot( Member* self, PyObject* args )
{
    if( PyTuple_GET_SIZE( args ) != 2 )
        return py_type_fail( "set_slot() takes exactly 2 arguments" );
    PyObject* object = PyTuple_GET_ITEM( args, 0 );
    PyObject* value = PyTuple_GET_ITEM( args, 1 );
    if( !CAtom::TypeCheck( object ) )
        return py_expected_type_fail( object, "CAtom" );
    CAtom* atom = catom_cast( object );
    if( self->index >= atom->get_slot_count() )
        return py_no_attr_fail( object, (char *)Py23Str_AS_STRING( self->name ) );
    atom->set_slot( self->index, value );
    Py_RETURN_NONE;
}


static PyObject*
Member_del_slot( Member* self, PyObject* object )
{
    if( !CAtom::TypeCheck( object ) )
        return py_expected_type_fail( object, "CAtom" );
    CAtom* atom = catom_cast( object );
    if( self->index >= atom->get_slot_count() )
        return py_no_attr_fail( object, (char *)Py23Str_AS_STRING( self->name ) );
    atom->set_slot( self->index, 0 );
    Py_RETURN_NONE;
}


static PyObject*
Member_do_getattr( Member* self, PyObject* object )
{
    if( !CAtom::TypeCheck( object ) )
        return py_expected_type_fail( object, "CAtom" );
    return self->getattr( catom_cast( object ) );
}


static PyObject*
Member_do_setattr( Member* self, PyObject* args )
{
    if( PyTuple_GET_SIZE( args ) != 2 )
        return py_type_fail( "do_setattr() takes exactly 2 arguments" );
    PyObject* object = PyTuple_GET_ITEM( args, 0 );
    PyObject* value = PyTuple_GET_ITEM( args, 1 );
    if( !CAtom::TypeCheck( object ) )
        return py_expected_type_fail( object, "CAtom" );
    if( self->setattr( catom_cast( object ), value ) < 0 )
        return 0;
    Py_RETURN_NONE;
}


static PyObject*
Member_do_delattr( Member* self, PyObject* object )
{
    if( !CAtom::TypeCheck( object ) )
        return py_expected_type_fail( object, "CAtom" );
    if( self->delattr( catom_cast( object ) ) < 0 )
        return 0;
    Py_RETURN_NONE;
}


static PyObject*
Member_do_post_getattr( Member* self, PyObject* args )
{
    if( PyTuple_GET_SIZE( args ) != 2 )
        return py_type_fail( "do_post_getattr() takes exactly 2 arguments" );
    PyObject* object = PyTuple_GET_ITEM( args, 0 );
    PyObject* value = PyTuple_GET_ITEM( args, 1 );
    if( !CAtom::TypeCheck( object ) )
        return py_expected_type_fail( object, "CAtom" );
    return self->post_getattr( catom_cast( object ), value );
}


static PyObject*
Member_do_post_setattr( Member* self, PyObject* args )
{
    if( PyTuple_GET_SIZE( args ) != 3 )
        return py_type_fail( "do_post_setattr() takes exactly 3 arguments" );
    PyObject* object = PyTuple_GET_ITEM( args, 0 );
    PyObject* oldvalue = PyTuple_GET_ITEM( args, 1 );
    PyObject* newvalue = PyTuple_GET_ITEM( args, 2 );
    if( !CAtom::TypeCheck( object ) )
        return py_expected_type_fail( object, "CAtom" );
    if( self->post_setattr( catom_cast( object ), oldvalue, newvalue ) < 0 )
        return 0;
    Py_RETURN_NONE;
}


static PyObject*
Member_do_default_value( Member* self, PyObject* object )
{
    if( !CAtom::TypeCheck( object ) )
        return py_expected_type_fail( object, "CAtom" );
    return self->default_value( catom_cast( object ) );
}


static PyObject*
Member_do_validate( Member* self, PyObject* args )
{
    if( PyTuple_GET_SIZE( args ) != 3 )
        return py_type_fail( "do_validate() takes exactly 3 arguments" );
    PyObject* object = PyTuple_GET_ITEM( args, 0 );
    PyObject* oldvalue = PyTuple_GET_ITEM( args, 1 );
    PyObject* newvalue = PyTuple_GET_ITEM( args, 2 );
    if( !CAtom::TypeCheck( object ) )
        return py_expected_type_fail( object, "CAtom" );
    return self->validate( catom_cast( object ), oldvalue, newvalue );
}


static PyObject*
Member_do_post_validate( Member* self, PyObject* args )
{
    if( PyTuple_GET_SIZE( args ) != 3 )
        return py_type_fail( "do_post_validate() takes exactly 3 arguments" );
    PyObject* object = PyTuple_GET_ITEM( args, 0 );
    PyObject* oldvalue = PyTuple_GET_ITEM( args, 1 );
    PyObject* newvalue = PyTuple_GET_ITEM( args, 2 );
    if( !CAtom::TypeCheck( object ) )
        return py_expected_type_fail( object, "CAtom" );
    return self->post_validate( catom_cast( object ), oldvalue, newvalue );
}


static PyObject*
Member_do_full_validate( Member* self, PyObject* args )
{
    if( PyTuple_GET_SIZE( args ) != 3 )
        return py_type_fail( "do_full_validate() takes exactly 3 arguments" );
    PyObject* object = PyTuple_GET_ITEM( args, 0 );
    PyObject* oldvalue = PyTuple_GET_ITEM( args, 1 );
    PyObject* newvalue = PyTuple_GET_ITEM( args, 2 );
    if( !CAtom::TypeCheck( object ) )
        return py_expected_type_fail( object, "CAtom" );
    return self->full_validate( catom_cast( object ), oldvalue, newvalue );
}


static PyObject*
Member_clone( Member* self )
{
    // reimplement in a subclass to clone additional Python state
    PyObject* pyclone = PyType_GenericNew( Py_TYPE(self), 0, 0 );
    if( !pyclone )
        return 0;
    Member* clone = member_cast( pyclone );
    clone->modes = self->modes;
    clone->index = self->index;
    clone->name = newref( self->name );
    if( self->metadata )
        clone->metadata = PyDict_Copy( self->metadata );
    clone->getattr_context = xnewref( self->getattr_context );
    clone->setattr_context = xnewref( self->setattr_context );
    clone->delattr_context = xnewref( self->delattr_context );
    clone->validate_context = xnewref( self->validate_context );
    clone->post_getattr_context = xnewref( self->post_getattr_context );
    clone->post_setattr_context = xnewref( self->post_setattr_context );
    clone->default_value_context = xnewref( self->default_value_context );
    clone->post_validate_context = xnewref( self->post_validate_context );
    if( self->static_observers )
    {
        clone->static_observers = new std::vector<PyObjectPtr>();
        *clone->static_observers = *self->static_observers;
    }
    return pyclone;
}


static PyObject*
Member_get_name( Member* self, void* context )
{
    return newref( self->name );
}


static PyObject*
Member_set_name( Member* self, PyObject* value )
{
    if( !Py23Str_CheckExact( value ) )
        return py_expected_type_fail( value, "str" );
    Py_INCREF( value ); // incref before interning or segfault!
    Py23Str_InternInPlace( &value );
    PyObject* old = self->name;
    self->name = value;
    Py_DECREF( old );
    Py_RETURN_NONE;
}


static PyObject*
Member_get_index( Member* self, void* context )
{
    return Py23Int_FromSsize_t( static_cast<Py_ssize_t>( self->index ) );
}


static PyObject*
Member_set_index( Member* self, PyObject* value )
{
    if( !Py23Int_Check( value ) )
        return py_expected_type_fail( value, "int" );
    Py_ssize_t index = Py23Int_AsSsize_t( value );
    if( index < 0 && PyErr_Occurred() )
        return 0;
    self->index = static_cast<uint32_t>( index < 0 ? 0 : index );
    Py_RETURN_NONE;
}


template<typename T> bool
parse_mode_and_context( PyObject* args, PyObject** context, T& mode )
{
    PyObject* pymode;
    if( !PyArg_ParseTuple( args, "OO", &pymode, context ) )
        return false;
    if( !EnumTypes::from_py_enum( pymode, mode ) )
        return false;
    if( !Member::check_context( mode, *context ) )
        return false;
    return true;
}


static PyObject*
Member_get_getattr_mode( Member* self, void* ctxt )
{
    PyTuplePtr tuple( PyTuple_New( 2 ) );
    if( !tuple )
        return 0;
    PyObjectPtr py_enum( EnumTypes::to_py_enum( self->get_getattr_mode() ) );
    if( !py_enum )
        return 0;
    tuple.set_item( 0, py_enum );
    PyObject* context = self->getattr_context;
    tuple.set_item( 1, newref( context ? context : Py_None ) );
    return tuple.release();
}


static PyObject*
Member_set_getattr_mode( Member* self, PyObject* args )
{
    GetAttr::Mode mode;
    PyObject* context;
    if( !parse_mode_and_context( args, &context, mode ) )
        return 0;
    self->set_getattr_mode( mode );
    PyObject* old = self->getattr_context;
    self->getattr_context = context;
    Py_INCREF( context );
    Py_XDECREF( old );
    Py_RETURN_NONE;
}


static PyObject*
Member_get_setattr_mode( Member* self, void* ctxt )
{
    PyTuplePtr tuple( PyTuple_New( 2 ) );
    if( !tuple )
        return 0;
    PyObjectPtr py_enum( EnumTypes::to_py_enum( self->get_setattr_mode() ) );
    if( !py_enum )
        return 0;
    tuple.set_item( 0, py_enum );
    PyObject* context = self->setattr_context;
    tuple.set_item( 1, newref( context ? context : Py_None ) );
    return tuple.release();
}


static PyObject*
Member_set_setattr_mode( Member* self, PyObject* args )
{
    SetAttr::Mode mode;
    PyObject* context;
    if( !parse_mode_and_context( args, &context, mode ) )
        return 0;
    self->set_setattr_mode( mode );
    PyObject* old = self->setattr_context;
    self->setattr_context = context;
    Py_INCREF( context );
    Py_XDECREF( old );
    Py_RETURN_NONE;
}


static PyObject*
Member_get_delattr_mode( Member* self, void* ctxt )
{
    PyTuplePtr tuple( PyTuple_New( 2 ) );
    if( !tuple )
        return 0;
    PyObjectPtr py_enum( EnumTypes::to_py_enum( self->get_delattr_mode() ) );
    if( !py_enum )
        return 0;
    tuple.set_item( 0, py_enum );
    PyObject* context = self->delattr_context;
    tuple.set_item( 1, newref( context ? context : Py_None ) );
    return tuple.release();
}


static PyObject*
Member_set_delattr_mode( Member* self, PyObject* args )
{
    DelAttr::Mode mode;
    PyObject* context;
    if( !parse_mode_and_context( args, &context, mode ) )
        return 0;
    self->set_delattr_mode( mode );
    PyObject* old = self->delattr_context;
    self->delattr_context = context;
    Py_INCREF( context );
    Py_XDECREF( old );
    Py_RETURN_NONE;
}


static PyObject*
Member_get_post_getattr_mode( Member* self, void* ctxt )
{
    PyTuplePtr tuple( PyTuple_New( 2 ) );
    if( !tuple )
        return 0;
    PyObjectPtr py_enum( EnumTypes::to_py_enum( self->get_post_getattr_mode() ) );
    if( !py_enum )
        return 0;
    tuple.set_item( 0, py_enum );
    PyObject* context = self->post_getattr_context;
    tuple.set_item( 1, newref( context ? context : Py_None ) );
    return tuple.release();
}


static PyObject*
Member_set_post_getattr_mode( Member* self, PyObject* args )
{
    PostGetAttr::Mode mode;
    PyObject* context;
    if( !parse_mode_and_context( args, &context, mode ) )
        return 0;
    self->set_post_getattr_mode( mode );
    PyObject* old = self->post_getattr_context;
    self->post_getattr_context = context;
    Py_INCREF( context );
    Py_XDECREF( old );
    Py_RETURN_NONE;
}


static PyObject*
Member_get_post_setattr_mode( Member* self, void* ctxt )
{
    PyTuplePtr tuple( PyTuple_New( 2 ) );
    if( !tuple )
        return 0;
    PyObjectPtr py_enum( EnumTypes::to_py_enum( self->get_post_setattr_mode() ) );
    if( !py_enum )
        return 0;
    tuple.set_item( 0, py_enum );
    PyObject* context = self->post_setattr_context;
    tuple.set_item( 1, newref( context ? context : Py_None ) );
    return tuple.release();
}


static PyObject*
Member_set_post_setattr_mode( Member* self, PyObject* args )
{
    PostSetAttr::Mode mode;
    PyObject* context;
    if( !parse_mode_and_context( args, &context, mode ) )
        return 0;
    self->set_post_setattr_mode( mode );
    PyObject* old = self->post_setattr_context;
    self->post_setattr_context = context;
    Py_INCREF( context );
    Py_XDECREF( old );
    Py_RETURN_NONE;
}


static PyObject*
Member_get_default_value_mode( Member* self, void* ctxt )
{
    PyTuplePtr tuple( PyTuple_New( 2 ) );
    if( !tuple )
        return 0;
    PyObjectPtr py_enum( EnumTypes::to_py_enum( self->get_default_value_mode() ) );
    if( !py_enum )
        return 0;
    tuple.set_item( 0, py_enum );
    PyObject* context = self->default_value_context;
    tuple.set_item( 1, newref( context ? context : Py_None ) );
    return tuple.release();
}


static PyObject*
Member_set_default_value_mode( Member* self, PyObject* args )
{
    DefaultValue::Mode mode;
    PyObject* context;
    if( !parse_mode_and_context( args, &context, mode ) )
        return 0;
    self->set_default_value_mode( mode );
    PyObject* old = self->default_value_context;
    self->default_value_context = context;
    Py_INCREF( context );
    Py_XDECREF( old );
    Py_RETURN_NONE;
}


static PyObject*
Member_get_validate_mode( Member* self, void* ctxt )
{
    PyTuplePtr tuple( PyTuple_New( 2 ) );
    if( !tuple )
        return 0;
    PyObjectPtr py_enum( EnumTypes::to_py_enum( self->get_validate_mode() ) );
    if( !py_enum )
        return 0;
    tuple.set_item( 0, py_enum );
    PyObject* context = self->validate_context;
    tuple.set_item( 1, newref( context ? context : Py_None ) );
    return tuple.release();
}


static PyObject*
Member_set_validate_mode( Member* self, PyObject* args )
{
    Validate::Mode mode;
    PyObject* context;
    if( !parse_mode_and_context( args, &context, mode ) )
        return 0;
    self->set_validate_mode( mode );
    PyObject* old = self->validate_context;
    self->validate_context = context;
    Py_INCREF( context );
    Py_XDECREF( old );
    Py_RETURN_NONE;
}


static PyObject*
Member_get_post_validate_mode( Member* self, void* ctxt )
{
    PyTuplePtr tuple( PyTuple_New( 2 ) );
    if( !tuple )
        return 0;
    PyObjectPtr py_enum( EnumTypes::to_py_enum( self->get_post_validate_mode() ) );
    if( !py_enum )
        return 0;
    tuple.set_item( 0, py_enum );
    PyObject* context = self->post_validate_context;
    tuple.set_item( 1, newref( context ? context : Py_None ) );
    return tuple.release();
}


static PyObject*
Member_set_post_validate_mode( Member* self, PyObject* args )
{
    PostValidate::Mode mode;
    PyObject* context;
    if( !parse_mode_and_context( args, &context, mode ) )
        return 0;
    self->set_post_validate_mode( mode );
    PyObject* old = self->post_validate_context;
    self->post_validate_context = context;
    Py_INCREF( context );
    Py_XDECREF( old );
    Py_RETURN_NONE;
}


static PyObject*
Member_notify( Member* self, PyObject* args, PyObject* kwargs )
{
    if( PyTuple_GET_SIZE( args ) < 1 )
        return py_type_fail( "notify() requires at least 1 argument" );
    PyObject* owner = PyTuple_GET_ITEM( args, 0 );
    if( !CAtom::TypeCheck( owner ) )
        return py_expected_type_fail( owner, "CAtom" );
    PyObjectPtr argsptr( PyTuple_GetSlice( args, 1, PyTuple_GET_SIZE( args ) ) );
    if( !argsptr )
        return 0;
    if( !self->notify( catom_cast( owner ), argsptr.get(), kwargs ) )
        return 0;
    Py_RETURN_NONE;
}


static PyObject*
Member_tag( Member* self, PyObject* args, PyObject* kwargs )
{
    if( PyTuple_GET_SIZE( args ) != 0 )
        return py_type_fail( "tag() takes no positional arguments" );
    if( !kwargs )
        return py_type_fail( "tag() requires keyword arguments" );
    if( !self->metadata )
    {
        self->metadata = PyDict_New();
        if( !self->metadata )
            return 0;
    }
    if( PyDict_Update( self->metadata, kwargs ) < 0 )
        return 0;
    return newref( pyobject_cast( self ) );
}


static PyObject*
Member_get_metadata( Member* self, void* ctxt )
{
    if( !self->metadata )
        Py_RETURN_NONE;
    Py_INCREF( self->metadata );
    return self->metadata;
}


static int
Member_set_metadata( Member* self, PyObject* value, void* ctxt )
{
    if( value && value != Py_None && !PyDict_Check( value ) )
    {
        py_expected_type_fail( value, "dict or None" );
        return -1;
    }
    if( value == Py_None )
        value = 0;
    PyObject* old = self->metadata;
    self->metadata = value;
    Py_XINCREF( value );
    Py_XDECREF( old );
    return 0;
}


static PyObject*
Member__get__( Member* self, PyObject* object, PyObject* type )
{
    if( !object )
        return newref( pyobject_cast( self ) );
    if( !CAtom::TypeCheck( object ) )
        return py_expected_type_fail( object, "CAtom" );
    return self->getattr( catom_cast( object ) );
}


static int
Member__set__( Member* self, PyObject* object, PyObject* value )
{
    if( !CAtom::TypeCheck( object ) )
    {
        py_expected_type_fail( object, "CAtom" );
        return -1;
    }
    if( value )
        return self->setattr( catom_cast( object ), value );
    return self->delattr( catom_cast( object ) );
}


static PyGetSetDef
Member_getset[] = {
    { "name", ( getter )Member_get_name, 0,
      "Get the name to which the member is bound." },
    { "metadata", ( getter )Member_get_metadata, ( setter )Member_set_metadata,
      "Get and set the metadata for the member." },
    { "index", ( getter )Member_get_index, 0,
      "Get the index to which the member is bound" },
    { "getattr_mode", ( getter )Member_get_getattr_mode, 0,
      "Get the getattr mode for the member." },
    { "setattr_mode", ( getter )Member_get_setattr_mode, 0,
      "Get the setattr mode for the member." },
    { "delattr_mode", ( getter )Member_get_delattr_mode, 0,
      "Get the delattr mode for the member." },
    { "default_value_mode", ( getter )Member_get_default_value_mode, 0,
      "Get the default value mode for the member." },
    { "validate_mode", ( getter )Member_get_validate_mode, 0,
      "Get the validate mode for the member." },
    { "post_getattr_mode", ( getter )Member_get_post_getattr_mode, 0,
      "Get the post getattr mode for the member." },
    { "post_setattr_mode", ( getter )Member_get_post_setattr_mode, 0,
      "Get the post setattr mode for the member." },
    { "post_validate_mode", ( getter )Member_get_post_validate_mode, 0,
      "Get the post validate mode for the member." },
    { 0 } // sentinel
};


static PyMethodDef
Member_methods[] = {
    { "set_name", ( PyCFunction )Member_set_name, METH_O,
      "Set the name to which the member is bound. Use with extreme caution!" },
    { "set_index", ( PyCFunction )Member_set_index, METH_O,
      "Set the index to which the member is bound. Use with extreme caution!" },
    { "get_slot", ( PyCFunction )Member_get_slot, METH_O,
      "Get the atom's slot value directly." },
    { "set_slot", ( PyCFunction )Member_set_slot, METH_VARARGS,
      "Set the atom's slot value directly." },
    { "del_slot", ( PyCFunction )Member_del_slot, METH_O,
      "Delete the atom's slot value directly." },
    { "has_observers", ( PyCFunction )Member_has_observers, METH_NOARGS,
      "Get whether or not this member has observers." },
    { "has_observer", ( PyCFunction )Member_has_observer, METH_O,
      "Get whether or not the member already has the given observer." },
    { "copy_static_observers", ( PyCFunction )Member_copy_static_observers, METH_O,
      "Copy the static observers from one member into this member." },
    { "static_observers", ( PyCFunction )Member_static_observers, METH_NOARGS,
      "Get a tuple of the static observers defined for this member" },
    { "add_static_observer", ( PyCFunction )Member_add_static_observer, METH_O,
      "Add the name of a method to call on all atoms when the member changes." },
    { "remove_static_observer", ( PyCFunction )Member_remove_static_observer, METH_O,
      "Remove the name of a method to call on all atoms when the member changes." },
    { "clone", ( PyCFunction )Member_clone, METH_NOARGS,
      "Create a clone of this member." },
    { "do_getattr", ( PyCFunction )Member_do_getattr, METH_O,
      "Run the getattr handler for the member." },
    { "do_setattr", ( PyCFunction )Member_do_setattr, METH_VARARGS,
      "Run the setattr handler for the member." },
    { "do_delattr", ( PyCFunction )Member_do_delattr, METH_O,
      "Run the delattr handler for the member." },
    { "do_default_value", ( PyCFunction )Member_do_default_value, METH_O,
      "Run the default value handler for member." },
    { "do_validate", ( PyCFunction )Member_do_validate, METH_VARARGS,
      "Run the validation handler for the member." },
    { "do_post_getattr", ( PyCFunction )Member_do_post_getattr, METH_VARARGS,
      "Run the post getattr handler for the member." },
    { "do_post_setattr", ( PyCFunction )Member_do_post_setattr, METH_VARARGS,
      "Run the post setattr handler for the member." },
    { "do_post_validate", ( PyCFunction )Member_do_post_validate, METH_VARARGS,
      "Run the post validation handler for the member." },
    { "do_full_validate", ( PyCFunction )Member_do_full_validate, METH_VARARGS,
      "Run the validation and post validation handlers for the member." },
    { "set_getattr_mode", ( PyCFunction )Member_set_getattr_mode, METH_VARARGS,
      "Set the getattr mode for the member." },
    { "set_setattr_mode", ( PyCFunction )Member_set_setattr_mode, METH_VARARGS,
      "Set the setattr mode for the member." },
    { "set_delattr_mode", ( PyCFunction )Member_set_delattr_mode, METH_VARARGS,
      "Set the delattr mode for the member." },
    { "set_default_value_mode", ( PyCFunction )Member_set_default_value_mode, METH_VARARGS,
      "Set the default value mode for the member." },
    { "set_validate_mode", ( PyCFunction )Member_set_validate_mode, METH_VARARGS,
      "Set the validate mode for the member." },
    { "set_post_getattr_mode", ( PyCFunction )Member_set_post_getattr_mode, METH_VARARGS,
      "Set the post getattr mode for the member." },
    { "set_post_setattr_mode", ( PyCFunction )Member_set_post_setattr_mode, METH_VARARGS,
      "Set the post setattr mode for the member." },
    { "set_post_validate_mode", ( PyCFunction )Member_set_post_validate_mode, METH_VARARGS,
      "Set the post validate mode for the member." },
    { "notify", ( PyCFunction )Member_notify, METH_VARARGS | METH_KEYWORDS,
      "Notify the static observers for the given member and atom." },
    { "tag", ( PyCFunction )Member_tag, METH_VARARGS | METH_KEYWORDS,
      "Tag the member with metatdata. " },
    { 0 } // sentinel
};


PyTypeObject Member_Type = {
    PyVarObject_HEAD_INIT( &PyType_Type, 0 )
    PACKAGE_TYPENAME( "Member" ),           /* tp_name */
    sizeof( Member ),                       /* tp_basicsize */
    0,                                      /* tp_itemsize */
    (destructor)Member_dealloc,             /* tp_dealloc */
    (printfunc)0,                           /* tp_print */
    (getattrfunc)0,                         /* tp_getattr */
    (setattrfunc)0,                         /* tp_setattr */
#if PY_MAJOR_VERSION >= 3
#if PY_MINOR_VERSION > 4
    ( PyAsyncMethods* )0,                  /* tp_as_async */
#else
    ( void* ) 0,                           /* tp_reserved */
#endif
#else
    ( cmpfunc )0,                          /* tp_compare */
#endif
    (reprfunc)0,                            /* tp_repr */
    (PyNumberMethods*)0,                    /* tp_as_number */
    (PySequenceMethods*)0,                  /* tp_as_sequence */
    (PyMappingMethods*)0,                   /* tp_as_mapping */
    (hashfunc)0,                            /* tp_hash */
    (ternaryfunc)0,                         /* tp_call */
    (reprfunc)0,                            /* tp_str */
    (getattrofunc)0,                        /* tp_getattro */
    (setattrofunc)0,                        /* tp_setattro */
    (PyBufferProcs*)0,                      /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE|Py_TPFLAGS_HAVE_GC, /* tp_flags */
    0,                                      /* Documentation string */
    (traverseproc)Member_traverse,          /* tp_traverse */
    (inquiry)Member_clear,                  /* tp_clear */
    (richcmpfunc)0,                         /* tp_richcompare */
    0,                                      /* tp_weaklistoffset */
    (getiterfunc)0,                         /* tp_iter */
    (iternextfunc)0,                        /* tp_iternext */
    (struct PyMethodDef*)Member_methods,    /* tp_methods */
    (struct PyMemberDef*)0,                 /* tp_members */
    Member_getset,                          /* tp_getset */
    0,                                      /* tp_base */
    0,                                      /* tp_dict */
    (descrgetfunc)Member__get__,            /* tp_descr_get */
    (descrsetfunc)Member__set__,            /* tp_descr_set */
    0,                                      /* tp_dictoffset */
    (initproc)0,                            /* tp_init */
    (allocfunc)PyType_GenericAlloc,         /* tp_alloc */
    (newfunc)Member_new,                    /* tp_new */
    (freefunc)PyObject_GC_Del,              /* tp_free */
    (inquiry)0,                             /* tp_is_gc */
    0,                                      /* tp_bases */
    0,                                      /* tp_mro */
    0,                                      /* tp_cache */
    0,                                      /* tp_subclasses */
    0,                                      /* tp_weaklist */
    (destructor)0                           /* tp_del */
};


int
import_member()
{
    if( PyType_Ready( &Member_Type ) < 0 )
        return -1;
    undefined = Py23Str_FromString( "<undefined>" );
    if( !undefined )
        return -1;
    return 0;
}


PyObject*
Member::full_validate( CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    PyObjectPtr result( newref( newvalue ) );
    if( get_validate_mode() )
    {
        result = validate( atom, oldvalue, result.get() );
        if( !result )
            return 0;
    }
    if( get_post_validate_mode() )
    {
        result = post_validate( atom, oldvalue, result.get() );
        if( !result )
            return 0;
    }
    return result.release();
}


namespace
{

struct BaseTask : public ModifyTask
{
    BaseTask( Member* member, PyObject* observer ) :
        m_member( newref( pyobject_cast( member ) ) ),
        m_observer( newref( observer ) ) {}
    PyObjectPtr m_member;
    PyObjectPtr m_observer;
};


struct AddTask : public BaseTask
{
    AddTask( Member* member, PyObject* observer ) :
        BaseTask( member, observer ) {}
    void run()
    {
        Member* member = member_cast( m_member.get() );
        member->add_observer( m_observer.get() );
    }
};


struct RemoveTask : public BaseTask
{
    RemoveTask( Member* member, PyObject* observer ) :
        BaseTask( member, observer ) {}
    void run()
    {
        Member* member = member_cast( m_member.get() );
        member->remove_observer( m_observer.get() );
    }
};

} // namespace


void
Member::add_observer( PyObject* observer )
{
    if( modify_guard )
    {
        ModifyTask* task = new AddTask( this, observer );
        modify_guard->add_task( task );
        return;
    }
    if( !static_observers )
        static_observers = new std::vector<PyObjectPtr>();
    PyObjectPtr obptr( newref( observer ) );
    std::vector<PyObjectPtr>::iterator it;
    std::vector<PyObjectPtr>::iterator end = static_observers->end();
    for( it = static_observers->begin(); it != end; ++it )
    {
        if( *it == obptr || it->richcompare( obptr, Py_EQ ) )
            return;
    }
    static_observers->push_back( obptr );
    return;
}


void
Member::remove_observer( PyObject* observer )
{
    if( modify_guard )
    {
        ModifyTask* task = new RemoveTask( this, observer );
        modify_guard->add_task( task );
        return;
    }
    if( static_observers )
    {
        PyObjectPtr obptr( newref( observer ) );
        std::vector<PyObjectPtr>::iterator it;
        std::vector<PyObjectPtr>::iterator end = static_observers->end();
        for( it = static_observers->begin(); it != end; ++it )
        {
            if( *it == obptr || it->richcompare( obptr, Py_EQ ) )
            {
                static_observers->erase( it );
                if( static_observers->size() == 0 )
                {
                    delete static_observers;
                    static_observers = 0;
                }
                break;
            }
        }
    }
}


bool
Member::has_observer( PyObject* observer )
{
    if( !static_observers )
        return false;
    PyObjectPtr obptr( newref( observer ) );
    std::vector<PyObjectPtr>::iterator it;
    std::vector<PyObjectPtr>::iterator end = static_observers->end();
    for( it = static_observers->begin(); it != end; ++it )
    {
        if( *it == obptr || it->richcompare( obptr, Py_EQ ) )
            return true;
    }
    return false;
}


bool
Member::notify( CAtom* atom, PyObject* args, PyObject* kwargs )
{
    if( static_observers && atom->get_notifications_enabled() )
    {
        ModifyGuard<Member> guard( *this );
        PyObjectPtr argsptr( newref( args ) );
        PyObjectPtr kwargsptr( xnewref( kwargs ) );
        PyObjectPtr objectptr( newref( pyobject_cast( atom ) ) );
        PyObjectPtr callable;
        std::vector<PyObjectPtr>::iterator it;
        std::vector<PyObjectPtr>::iterator end = static_observers->end();
        for( it = static_observers->begin(); it != end; ++it )
        {
            if( Py23Str_CheckExact( it->get() ) )
            {
                callable = objectptr.getattr( *it );
                if( !callable )
                    return false;
            }
            else
            {
                callable = *it;
            }
            if( !callable( argsptr, kwargsptr ) )
                return false;
        }
    }
    return true;
}
