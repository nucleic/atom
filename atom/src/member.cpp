/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2023, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file LICENSE, distributed with this software.
|----------------------------------------------------------------------------*/
#ifdef __clang__
#pragma clang diagnostic ignored "-Wdeprecated-writable-strings"
#endif

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif

#include <cppy/cppy.h>
#include "member.h"
#include "enumtypes.h"
#include "packagenaming.h"
#include "utils.h"


namespace atom
{


namespace
{

static PyObject* undefined;


PyObject*
Member_new( PyTypeObject* type, PyObject* args, PyObject* kwargs )
{
    cppy::ptr selfptr( PyType_GenericNew( type, args, kwargs ) );
    if( !selfptr )
        return 0;
    Member* member = member_cast( selfptr.get() );
    member->name = cppy::incref( undefined );
    member->set_getattr_mode( GetAttr::Slot );
    member->set_setattr_mode( SetAttr::Slot );
    member->set_delattr_mode( DelAttr::Slot );
    return selfptr.release();
}


void
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
    Py_CLEAR( self->getstate_context );
    if( self->static_observers )
        self->static_observers->clear();
}


int
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
    Py_VISIT( self->getstate_context );
    if( self->static_observers )
    {
        std::vector<Observer>::iterator it;
        std::vector<Observer>::iterator end = self->static_observers->end();
        for( it = self->static_observers->begin(); it != end; ++it )
            Py_VISIT( it->m_observer.get() );
    }
#if PY_VERSION_HEX >= 0x03090000
    // This was not needed before Python 3.9 (Python issue 35810 and 40217)
    Py_VISIT(Py_TYPE(self));
#endif
    return 0;
}


void
Member_dealloc( Member* self )
{
    PyObject_GC_UnTrack( self );
    Member_clear( self );
    delete self->static_observers;
    self->static_observers = 0;
    Py_TYPE(self)->tp_free( pyobject_cast( self ) );
}


PyObject*
Member_has_observers( Member* self, PyObject* args )
{
    const size_t n = PyTuple_GET_SIZE( args );
    if( n == 0 )
        return utils::py_bool( self->has_observers() );
    if( n > 1 )
        return cppy::type_error( "has_observers() takes at most 1 argument" );
    PyObject* types = PyTuple_GET_ITEM( args, 0 );
    if( !PyLong_Check( types ) )
        return cppy::type_error( types, "int" );
    uint8_t change_types = PyLong_AsLong( types );
    return utils::py_bool( self->has_observers( change_types ) );
}


PyObject*
Member_has_observer( Member* self, PyObject* args )
{
    const size_t n = PyTuple_GET_SIZE( args );
    if( n < 1 || n > 2 )
        return cppy::type_error( "has_observer() expects a callable and an optional change type" );
    PyObject* observer = PyTuple_GET_ITEM( args, 0 );
    if( !PyUnicode_CheckExact( observer ) && !PyCallable_Check( observer ) )
        return cppy::type_error( observer, "str or callable" );

    uint8_t change_types = ChangeType::Any;
    if ( n == 2 )
    {
        PyObject* types = PyTuple_GET_ITEM( args, 1 );
        if( !PyLong_Check( types ) )
            return cppy::type_error( types, "int" );
        change_types = PyLong_AsLong( types ) & 0xFF;
    }
    return utils::py_bool( self->has_observer( observer, change_types ) );
}


PyObject*
Member_copy_static_observers( Member* self, PyObject* other )
{
    if( !Member::TypeCheck( other ) )
        return cppy::type_error( other, "Member" );
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
            self->static_observers = new std::vector<Observer>();
        *self->static_observers = *member->static_observers;
    }
    Py_RETURN_NONE;
}


PyObject*
Member_static_observers( Member* self )
{
    if( !self->static_observers )
        return PyTuple_New( 0 );
    std::vector<Observer>& observers( *self->static_observers );
    size_t size = observers.size();
    PyObject* items = PyTuple_New( size );
    if( !items )
        return 0;
    for( size_t i = 0; i < size; ++i )
        PyTuple_SET_ITEM( items, i, cppy::incref( observers[ i ].m_observer.get() ) );
    return items;
}


PyObject*
Member_add_static_observer( Member* self, PyObject* args)
{
    const size_t n = PyTuple_GET_SIZE( args );
    if( n < 1 )
        return cppy::type_error( "add_static_observer() requires at least 1 argument" );
    if( n > 2 )
        return cppy::type_error( "add_static_observer() takes at most 2 arugments" );
    PyObject* observer = PyTuple_GET_ITEM( args, 0 );
    if( !PyUnicode_CheckExact( observer ) && !PyCallable_Check( observer ) )
        return cppy::type_error( observer, "str or callable" );
    uint8_t change_types = ChangeType::Any;
    if(n == 2)
    {
        PyObject* types = PyTuple_GET_ITEM( args, 1 );
        if( !PyLong_Check( types ) )
            return cppy::type_error( types, "int" );
        change_types = PyLong_AsLong( types ) & 0xFF ;
    }
    self->add_observer( observer, change_types );
    Py_RETURN_NONE;
}


PyObject*
Member_remove_static_observer( Member* self, PyObject* observer )
{
    if( !PyUnicode_CheckExact( observer ) && !PyCallable_Check( observer ) )
        return cppy::type_error( observer, "str or callable" );
    self->remove_observer( observer );
    Py_RETURN_NONE;
}


PyObject*
Member_get_slot( Member* self, PyObject* object )
{
    if( !CAtom::TypeCheck( object ) )
        return cppy::type_error( object, "CAtom" );
    CAtom* atom = catom_cast( object );
    if( self->index >= atom->get_slot_count() )
        return cppy::attribute_error( object, (char *)PyUnicode_AsUTF8( self->name ) );
    cppy::ptr value( atom->get_slot( self->index ) );
    if( value )
        return value.release();
    Py_RETURN_NONE;
}


PyObject*
Member_set_slot( Member* self, PyObject* args )
{
    if( PyTuple_GET_SIZE( args ) != 2 )
        return cppy::type_error( "set_slot() takes exactly 2 arguments" );
    PyObject* object = PyTuple_GET_ITEM( args, 0 );
    PyObject* value = PyTuple_GET_ITEM( args, 1 );
    if( !CAtom::TypeCheck( object ) )
        return cppy::type_error( object, "CAtom" );
    CAtom* atom = catom_cast( object );
    if( self->index >= atom->get_slot_count() )
        return cppy::attribute_error( object, (char *)PyUnicode_AsUTF8( self->name ) );
    atom->set_slot( self->index, value );
    Py_RETURN_NONE;
}


PyObject*
Member_del_slot( Member* self, PyObject* object )
{
    if( !CAtom::TypeCheck( object ) )
        return cppy::type_error( object, "CAtom" );
    CAtom* atom = catom_cast( object );
    if( self->index >= atom->get_slot_count() )
        return cppy::attribute_error( object, (char *)PyUnicode_AsUTF8( self->name ) );
    atom->set_slot( self->index, 0 );
    Py_RETURN_NONE;
}


PyObject*
Member_do_getattr( Member* self, PyObject* object )
{
    if( !CAtom::TypeCheck( object ) )
        return cppy::type_error( object, "CAtom" );
    return self->getattr( catom_cast( object ) );
}


PyObject*
Member_do_setattr( Member* self, PyObject* args )
{
    if( PyTuple_GET_SIZE( args ) != 2 )
        return cppy::type_error( "do_setattr() takes exactly 2 arguments" );
    PyObject* object = PyTuple_GET_ITEM( args, 0 );
    PyObject* value = PyTuple_GET_ITEM( args, 1 );
    if( !CAtom::TypeCheck( object ) )
        return cppy::type_error( object, "CAtom" );
    if( self->setattr( catom_cast( object ), value ) < 0 )
        return 0;
    Py_RETURN_NONE;
}


PyObject*
Member_do_delattr( Member* self, PyObject* object )
{
    if( !CAtom::TypeCheck( object ) )
        return cppy::type_error( object, "CAtom" );
    if( self->delattr( catom_cast( object ) ) < 0 )
        return 0;
    Py_RETURN_NONE;
}


PyObject*
Member_do_post_getattr( Member* self, PyObject* args )
{
    if( PyTuple_GET_SIZE( args ) != 2 )
        return cppy::type_error( "do_post_getattr() takes exactly 2 arguments" );
    PyObject* object = PyTuple_GET_ITEM( args, 0 );
    PyObject* value = PyTuple_GET_ITEM( args, 1 );
    if( !CAtom::TypeCheck( object ) )
        return cppy::type_error( object, "CAtom" );
    return self->post_getattr( catom_cast( object ), value );
}


PyObject*
Member_do_post_setattr( Member* self, PyObject* args )
{
    if( PyTuple_GET_SIZE( args ) != 3 )
        return cppy::type_error( "do_post_setattr() takes exactly 3 arguments" );
    PyObject* object = PyTuple_GET_ITEM( args, 0 );
    PyObject* oldvalue = PyTuple_GET_ITEM( args, 1 );
    PyObject* newvalue = PyTuple_GET_ITEM( args, 2 );
    if( !CAtom::TypeCheck( object ) )
        return cppy::type_error( object, "CAtom" );
    if( self->post_setattr( catom_cast( object ), oldvalue, newvalue ) < 0 )
        return 0;
    Py_RETURN_NONE;
}


PyObject*
Member_do_default_value( Member* self, PyObject* object )
{
    if( !CAtom::TypeCheck( object ) )
        return cppy::type_error( object, "CAtom" );
    return self->default_value( catom_cast( object ) );
}


PyObject*
Member_do_validate( Member* self, PyObject* args )
{
    if( PyTuple_GET_SIZE( args ) != 3 )
        return cppy::type_error( "do_validate() takes exactly 3 arguments" );
    PyObject* object = PyTuple_GET_ITEM( args, 0 );
    PyObject* oldvalue = PyTuple_GET_ITEM( args, 1 );
    PyObject* newvalue = PyTuple_GET_ITEM( args, 2 );
    if( !CAtom::TypeCheck( object ) )
        return cppy::type_error( object, "CAtom" );
    return self->validate( catom_cast( object ), oldvalue, newvalue );
}


PyObject*
Member_do_post_validate( Member* self, PyObject* args )
{
    if( PyTuple_GET_SIZE( args ) != 3 )
        return cppy::type_error( "do_post_validate() takes exactly 3 arguments" );
    PyObject* object = PyTuple_GET_ITEM( args, 0 );
    PyObject* oldvalue = PyTuple_GET_ITEM( args, 1 );
    PyObject* newvalue = PyTuple_GET_ITEM( args, 2 );
    if( !CAtom::TypeCheck( object ) )
        return cppy::type_error( object, "CAtom" );
    return self->post_validate( catom_cast( object ), oldvalue, newvalue );
}


PyObject*
Member_do_full_validate( Member* self, PyObject* args )
{
    if( PyTuple_GET_SIZE( args ) != 3 )
        return cppy::type_error( "do_full_validate() takes exactly 3 arguments" );
    PyObject* object = PyTuple_GET_ITEM( args, 0 );
    PyObject* oldvalue = PyTuple_GET_ITEM( args, 1 );
    PyObject* newvalue = PyTuple_GET_ITEM( args, 2 );
    if( !CAtom::TypeCheck( object ) )
        return cppy::type_error( object, "CAtom" );
    return self->full_validate( catom_cast( object ), oldvalue, newvalue );
}


PyObject*
Member_do_should_getstate( Member* self, PyObject* object )
{
    if( !CAtom::TypeCheck( object ) )
        return cppy::type_error( object, "CAtom" );
    return self->should_getstate( catom_cast( object ) );
}


PyObject*
Member_clone( Member* self )
{
    // reimplement in a subclass to clone additional Python state
    PyObject* pyclone = PyType_GenericNew( Py_TYPE(self), 0, 0 );
    if( !pyclone )
        return 0;
    Member* clone = member_cast( pyclone );
    clone->modes = self->modes;
    clone->extra_modes = self->extra_modes;
    clone->index = self->index;
    clone->name = cppy::incref( self->name );
    if( self->metadata )
        clone->metadata = PyDict_Copy( self->metadata );
    clone->getattr_context = cppy::xincref( self->getattr_context );
    clone->setattr_context = cppy::xincref( self->setattr_context );
    clone->delattr_context = cppy::xincref( self->delattr_context );
    clone->validate_context = cppy::xincref( self->validate_context );
    clone->post_getattr_context = cppy::xincref( self->post_getattr_context );
    clone->post_setattr_context = cppy::xincref( self->post_setattr_context );
    clone->default_value_context = cppy::xincref( self->default_value_context );
    clone->post_validate_context = cppy::xincref( self->post_validate_context );
    clone->getstate_context = cppy::xincref( self->getstate_context );
    if( self->static_observers )
    {
        clone->static_observers = new std::vector<Observer>();
        *clone->static_observers = *self->static_observers;
    }
    return pyclone;
}


PyObject*
Member_get_name( Member* self, void* context )
{
    return cppy::incref( self->name );
}


PyObject*
Member_set_name( Member* self, PyObject* value )
{
    if( !PyUnicode_CheckExact( value ) )
        return cppy::type_error( value, "str" );
    cppy::incref( value ); // incref before interning or segfault!
    PyUnicode_InternInPlace( &value );
    PyObject* old = self->name;
    self->name = value;
    cppy::decref( old );
    Py_RETURN_NONE;
}


PyObject*
Member_get_index( Member* self, void* context )
{
    return PyLong_FromSsize_t( static_cast<Py_ssize_t>( self->index ) );
}


PyObject*
Member_set_index( Member* self, PyObject* value )
{
    if( !PyLong_Check( value ) )
        return cppy::type_error( value, "int" );
    Py_ssize_t index = PyLong_AsSsize_t( value );
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


PyObject*
Member_get_getattr_mode( Member* self, void* ctxt )
{
    cppy::ptr tuple( PyTuple_New( 2 ) );
    if( !tuple )
        return 0;
    cppy::ptr py_enum( EnumTypes::to_py_enum( self->get_getattr_mode() ) );
    if( !py_enum )
        return 0;
    PyTuple_SET_ITEM( tuple.get(), 0, py_enum.release() );
    PyObject* context = self->getattr_context;
    PyTuple_SET_ITEM( tuple.get(), 1, cppy::incref( context ? context : Py_None ) );
    return tuple.release();
}


PyObject*
Member_set_getattr_mode( Member* self, PyObject* args )
{
    GetAttr::Mode mode;
    PyObject* context;
    if( !parse_mode_and_context( args, &context, mode ) )
        return 0;
    self->set_getattr_mode( mode );
    PyObject* old = self->getattr_context;
    self->getattr_context = context;
    cppy::incref( context );
    cppy::xdecref( old );
    Py_RETURN_NONE;
}


PyObject*
Member_get_setattr_mode( Member* self, void* ctxt )
{
    cppy::ptr tuple( PyTuple_New( 2 ) );
    if( !tuple )
        return 0;
    cppy::ptr py_enum( EnumTypes::to_py_enum( self->get_setattr_mode() ) );
    if( !py_enum )
        return 0;
    PyTuple_SET_ITEM( tuple.get(), 0, py_enum.release() );
    PyObject* context = self->setattr_context;
    PyTuple_SET_ITEM( tuple.get(), 1, cppy::incref( context ? context : Py_None ) );
    return tuple.release();
}


PyObject*
Member_set_setattr_mode( Member* self, PyObject* args )
{
    SetAttr::Mode mode;
    PyObject* context;
    if( !parse_mode_and_context( args, &context, mode ) )
        return 0;
    self->set_setattr_mode( mode );
    PyObject* old = self->setattr_context;
    self->setattr_context = context;
    cppy::incref( context );
    cppy::xdecref( old );
    Py_RETURN_NONE;
}


PyObject*
Member_get_delattr_mode( Member* self, void* ctxt )
{
    cppy::ptr tuple( PyTuple_New( 2 ) );
    if( !tuple )
        return 0;
    cppy::ptr py_enum( EnumTypes::to_py_enum( self->get_delattr_mode() ) );
    if( !py_enum )
        return 0;
    PyTuple_SET_ITEM( tuple.get(), 0, py_enum.release() );
    PyObject* context = self->delattr_context;
    PyTuple_SET_ITEM( tuple.get(), 1, cppy::incref( context ? context : Py_None ) );
    return tuple.release();
}


PyObject*
Member_set_delattr_mode( Member* self, PyObject* args )
{
    DelAttr::Mode mode;
    PyObject* context;
    if( !parse_mode_and_context( args, &context, mode ) )
        return 0;
    self->set_delattr_mode( mode );
    PyObject* old = self->delattr_context;
    self->delattr_context = context;
    cppy::incref( context );
    cppy::xdecref( old );
    Py_RETURN_NONE;
}


PyObject*
Member_get_post_getattr_mode( Member* self, void* ctxt )
{
    cppy::ptr tuple( PyTuple_New( 2 ) );
    if( !tuple )
        return 0;
    cppy::ptr py_enum( EnumTypes::to_py_enum( self->get_post_getattr_mode() ) );
    if( !py_enum )
        return 0;
    PyTuple_SET_ITEM( tuple.get(), 0, py_enum.release() );
    PyObject* context = self->post_getattr_context;
    PyTuple_SET_ITEM( tuple.get(), 1, cppy::incref( context ? context : Py_None ) );
    return tuple.release();
}


PyObject*
Member_set_post_getattr_mode( Member* self, PyObject* args )
{
    PostGetAttr::Mode mode;
    PyObject* context;
    if( !parse_mode_and_context( args, &context, mode ) )
        return 0;
    self->set_post_getattr_mode( mode );
    PyObject* old = self->post_getattr_context;
    self->post_getattr_context = context;
    cppy::incref( context );
    cppy::xdecref( old );
    Py_RETURN_NONE;
}


PyObject*
Member_get_post_setattr_mode( Member* self, void* ctxt )
{
    cppy::ptr tuple( PyTuple_New( 2 ) );
    if( !tuple )
        return 0;
    cppy::ptr py_enum( EnumTypes::to_py_enum( self->get_post_setattr_mode() ) );
    if( !py_enum )
        return 0;
    PyTuple_SET_ITEM( tuple.get(), 0, py_enum.release() );
    PyObject* context = self->post_setattr_context;
    PyTuple_SET_ITEM( tuple.get(), 1, cppy::incref( context ? context : Py_None ) );
    return tuple.release();
}


PyObject*
Member_set_post_setattr_mode( Member* self, PyObject* args )
{
    PostSetAttr::Mode mode;
    PyObject* context;
    if( !parse_mode_and_context( args, &context, mode ) )
        return 0;
    self->set_post_setattr_mode( mode );
    PyObject* old = self->post_setattr_context;
    self->post_setattr_context = context;
    cppy::incref( context );
    cppy::xdecref( old );
    Py_RETURN_NONE;
}


PyObject*
Member_get_default_value_mode( Member* self, void* ctxt )
{
    cppy::ptr tuple( PyTuple_New( 2 ) );
    if( !tuple )
        return 0;
    cppy::ptr py_enum( EnumTypes::to_py_enum( self->get_default_value_mode() ) );
    if( !py_enum )
        return 0;
    PyTuple_SET_ITEM( tuple.get(), 0, py_enum.release() );
    PyObject* context = self->default_value_context;
    PyTuple_SET_ITEM( tuple.get(), 1, cppy::incref( context ? context : Py_None ) );
    return tuple.release();
}


PyObject*
Member_set_default_value_mode( Member* self, PyObject* args )
{
    DefaultValue::Mode mode;
    PyObject* context;
    if( !parse_mode_and_context( args, &context, mode ) )
        return 0;
    self->set_default_value_mode( mode );
    PyObject* old = self->default_value_context;
    self->default_value_context = context;
    cppy::incref( context );
    cppy::xdecref( old );
    Py_RETURN_NONE;
}


PyObject*
Member_get_validate_mode( Member* self, void* ctxt )
{
    cppy::ptr tuple( PyTuple_New( 2 ) );
    if( !tuple )
        return 0;
    cppy::ptr py_enum( EnumTypes::to_py_enum( self->get_validate_mode() ) );
    if( !py_enum )
        return 0;
    PyTuple_SET_ITEM( tuple.get(), 0, py_enum.release() );
    PyObject* context = self->validate_context;
    PyTuple_SET_ITEM( tuple.get(), 1, cppy::incref( context ? context : Py_None ) );
    return tuple.release();
}


PyObject*
Member_set_validate_mode( Member* self, PyObject* args )
{
    Validate::Mode mode;
    PyObject* context;
    if( !parse_mode_and_context( args, &context, mode ) )
        return 0;
    self->set_validate_mode( mode );
    PyObject* old = self->validate_context;
    self->validate_context = context;
    cppy::incref( context );
    cppy::xdecref( old );
    Py_RETURN_NONE;
}


PyObject*
Member_get_post_validate_mode( Member* self, void* ctxt )
{
    cppy::ptr tuple( PyTuple_New( 2 ) );
    if( !tuple )
        return 0;
    cppy::ptr py_enum( EnumTypes::to_py_enum( self->get_post_validate_mode() ) );
    if( !py_enum )
        return 0;
    PyTuple_SET_ITEM( tuple.get(), 0, py_enum.release() );
    PyObject* context = self->post_validate_context;
    PyTuple_SET_ITEM( tuple.get(), 1, cppy::incref( context ? context : Py_None ) );
    return tuple.release();
}


PyObject*
Member_set_post_validate_mode( Member* self, PyObject* args )
{
    PostValidate::Mode mode;
    PyObject* context;
    if( !parse_mode_and_context( args, &context, mode ) )
        return 0;
    self->set_post_validate_mode( mode );
    PyObject* old = self->post_validate_context;
    self->post_validate_context = context;
    cppy::incref( context );
    cppy::xdecref( old );
    Py_RETURN_NONE;
}


PyObject*
Member_get_getstate_mode( Member* self, void* ctxt )
{
    cppy::ptr tuple( PyTuple_New( 2 ) );
    if( !tuple )
        return 0;
    cppy::ptr py_enum( EnumTypes::to_py_enum( self->get_getstate_mode() ) );
    if( !py_enum )
        return 0;
    PyTuple_SET_ITEM( tuple.get(), 0, py_enum.release() );
    PyObject* context = self->getstate_context;
    PyTuple_SET_ITEM( tuple.get(), 1, cppy::incref( context ? context : Py_None ) );
    return tuple.release();
}


PyObject*
Member_set_getstate_mode( Member* self, PyObject* args )
{
    GetState::Mode mode;
    PyObject* context;
    if( !parse_mode_and_context( args, &context, mode ) )
        return 0;
    self->set_getstate_mode( mode );
    PyObject* old = self->getstate_context;
    self->getstate_context = context;
    cppy::incref( context );
    cppy::xdecref( old );
    Py_RETURN_NONE;
}


PyObject*
Member_notify( Member* self, PyObject* args, PyObject* kwargs )
{
    if( PyTuple_GET_SIZE( args ) < 1 )
        return cppy::type_error( "notify() requires at least 1 argument" );
    PyObject* owner = PyTuple_GET_ITEM( args, 0 );
    if( !CAtom::TypeCheck( owner ) )
        return cppy::type_error( owner, "CAtom" );
    cppy::ptr argsptr( PyTuple_GetSlice( args, 1, PyTuple_GET_SIZE( args ) ) );
    if( !argsptr )
        return 0;
    if( !self->notify( catom_cast( owner ), argsptr.get(), kwargs ) )
        return 0;
    Py_RETURN_NONE;
}


PyObject*
Member_tag( Member* self, PyObject* args, PyObject* kwargs )
{
    if( PyTuple_GET_SIZE( args ) != 0 )
        return cppy::type_error( "tag() takes no positional arguments" );
    if( !kwargs )
        return cppy::type_error( "tag() requires keyword arguments" );
    if( !self->metadata )
    {
        self->metadata = PyDict_New();
        if( !self->metadata )
            return 0;
    }
    if( PyDict_Update( self->metadata, kwargs ) < 0 )
        return 0;
    return cppy::incref( pyobject_cast( self ) );
}


PyObject*
Member_get_metadata( Member* self, void* ctxt )
{
    if( !self->metadata )
        Py_RETURN_NONE;
    cppy::incref( self->metadata );
    return self->metadata;
}


int
Member_set_metadata( Member* self, PyObject* value, void* ctxt )
{
    if( value && value != Py_None && !PyDict_Check( value ) )
    {
        cppy::type_error( value, "dict or None" );
        return -1;
    }
    if( value == Py_None )
        value = 0;
    PyObject* old = self->metadata;
    self->metadata = value;
    cppy::xincref( value );
    cppy::xdecref( old );
    return 0;
}


PyObject*
Member__get__( Member* self, PyObject* object, PyObject* type )
{
    if( !object )
        return cppy::incref( pyobject_cast( self ) );
    if( !CAtom::TypeCheck( object ) )
        return cppy::type_error( object, "CAtom" );
    return self->getattr( catom_cast( object ) );
}


int
Member__set__( Member* self, PyObject* object, PyObject* value )
{
    if( !CAtom::TypeCheck( object ) )
    {
        cppy::type_error( object, "CAtom" );
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
    { "getstate_mode", ( getter )Member_get_getstate_mode, 0,
      "Get the getstate mode for the member"},
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
    { "has_observers", ( PyCFunction )Member_has_observers, METH_VARARGS,
      "Get whether or not this member has observers." },
    { "has_observer", ( PyCFunction )Member_has_observer, METH_VARARGS,
      "Get whether or not the member already has the given observer." },
    { "copy_static_observers", ( PyCFunction )Member_copy_static_observers, METH_O,
      "Copy the static observers from one member into this member." },
    { "static_observers", ( PyCFunction )Member_static_observers, METH_NOARGS,
      "Get a tuple of the static observers defined for this member" },
    { "add_static_observer", ( PyCFunction )Member_add_static_observer, METH_VARARGS,
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
    { "do_should_getstate", ( PyCFunction )Member_do_should_getstate, METH_O,
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
    { "set_getstate_mode", ( PyCFunction )Member_set_getstate_mode, METH_VARARGS,
      "Set the getstate mode for the member." },
    { "notify", ( PyCFunction )Member_notify, METH_VARARGS | METH_KEYWORDS,
      "Notify the static observers for the given member and atom." },
    { "tag", ( PyCFunction )Member_tag, METH_VARARGS | METH_KEYWORDS,
      "Tag the member with metatdata. " },
#if PY_VERSION_HEX >= 0x03090000
    // Generic aliases have been added in 3.9 and allow to index types
    // This removes the need to quote explicit member type annotations
    { "__class_getitem__", (PyCFunction)Py_GenericAlias, METH_O|METH_CLASS,
      "See PEP 585"},
#endif
    { 0 } // sentinel
};


static PyType_Slot Member_Type_slots[] = {
    { Py_tp_dealloc, void_cast( Member_dealloc ) },              /* tp_dealloc */
    { Py_tp_traverse, void_cast( Member_traverse ) },            /* tp_traverse */
    { Py_tp_clear, void_cast( Member_clear ) },                  /* tp_clear */
    { Py_tp_methods, void_cast( Member_methods ) },              /* tp_methods */
    { Py_tp_getset, void_cast( Member_getset ) },                /* tp_getset */
    { Py_tp_descr_get, void_cast( Member__get__ ) },             /* tp_descr_get */
    { Py_tp_descr_set, void_cast( Member__set__ ) },             /* tp_descr_get */
    { Py_tp_new, void_cast( Member_new ) },                      /* tp_new */
    { Py_tp_alloc, void_cast( PyType_GenericAlloc ) },           /* tp_new */
    { Py_tp_free, void_cast( PyObject_GC_Del ) },                /* tp_new */
    { 0, 0 },
};


}  // namespace


// Initialize static variables (otherwise the compiler eliminate them)
PyTypeObject* Member::TypeObject = NULL;


PyType_Spec Member::TypeObject_Spec = {
	PACKAGE_TYPENAME( "Member" ),               /* tp_name */
	sizeof( Member ),                           /* tp_basicsize */
	0,                                          /* tp_itemsize */
	Py_TPFLAGS_DEFAULT
    |Py_TPFLAGS_BASETYPE
    |Py_TPFLAGS_HAVE_GC,                        /* tp_flags */
    Member_Type_slots                         /* slots */
};


PyObject*
Member::full_validate( CAtom* atom, PyObject* oldvalue, PyObject* newvalue )
{
    cppy::ptr result( cppy::incref( newvalue ) );
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
        m_member( cppy::incref( pyobject_cast( member ) ) ),
        m_observer( cppy::incref( observer ) ) {}
    cppy::ptr m_member;
    cppy::ptr m_observer;
};


struct AddTask : public BaseTask
{
    AddTask( Member* member, PyObject* observer, uint8_t change_types ) :
        BaseTask( member, observer ), m_change_types(change_types) {}
    void run()
    {
        Member* member = member_cast( m_member.get() );
        member->add_observer( m_observer.get(), m_change_types );
    }
    uint8_t m_change_types;
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

bool Member::has_observers( uint8_t change_types )
{
    if ( static_observers ) {
        std::vector<Observer>::iterator it;
        std::vector<Observer>::iterator end = static_observers->end();
        for( it = static_observers->begin(); it != end; ++it )
        {
            if( it->enabled( change_types ) )
                return true;
        }
    }
    return false;
}

void
Member::add_observer( PyObject* observer, uint8_t change_types )
{
    if( modify_guard )
    {
        ModifyTask* task = new AddTask( this, observer, change_types );
        modify_guard->add_task( task );
        return;
    }
    if( !static_observers )
        static_observers = new std::vector<Observer>();
    cppy::ptr obptr( cppy::incref( observer ) );
    std::vector<Observer>::iterator it;
    std::vector<Observer>::iterator end = static_observers->end();
    for( it = static_observers->begin(); it != end; ++it )
    {
        if( it->match( obptr ) )
        {
            it->m_change_types = change_types;
            return;
        }
    }
    static_observers->push_back( Observer(obptr, change_types) );
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
        cppy::ptr obptr( cppy::incref( observer ) );
        std::vector<Observer>::iterator it;
        std::vector<Observer>::iterator end = static_observers->end();
        for( it = static_observers->begin(); it != end; ++it )
        {
            if( it->match( obptr ) )
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
Member::has_observer( PyObject* observer, uint8_t change_types )
{
    if( !static_observers )
        return false;
    cppy::ptr obptr( cppy::incref( observer ) );
    std::vector<Observer>::iterator it;
    std::vector<Observer>::iterator end = static_observers->end();
    for( it = static_observers->begin(); it != end; ++it )
    {
        if( it->match( obptr ) && it->enabled( change_types ))
            return true;
    }
    return false;
}


bool
Member::notify( CAtom* atom, PyObject* args, PyObject* kwargs, uint8_t change_types)
{
    if( static_observers && atom->get_notifications_enabled() )
    {
        ModifyGuard<Member> guard( *this );
        cppy::ptr argsptr( cppy::incref( args ) );
        cppy::ptr kwargsptr( cppy::xincref( kwargs ) );
        cppy::ptr objectptr( cppy::incref( pyobject_cast( atom ) ) );
        cppy::ptr callable;
        std::vector<Observer>::iterator it;
        std::vector<Observer>::iterator end = static_observers->end();
        for( it = static_observers->begin(); it != end; ++it )
        {
            if ( !it->enabled( change_types ) )
                continue;  // Ignore

            if( PyUnicode_CheckExact( it->m_observer.get() ) )
            {
                callable = objectptr.getattr( it->m_observer );
                if( !callable )
                    return false;
            }
            else
            {
                callable = it->m_observer;
            }
            if( !callable.call( argsptr, kwargsptr ) )
                return false;
        }
    }
    return true;
}

bool Member::Ready()
{
    // The reference will be handled by the module to which we will add the type
	TypeObject = pytype_cast( PyType_FromSpec( &TypeObject_Spec ) );
    if( !TypeObject )
    {
        return false;
    }

    undefined = PyUnicode_FromString( "<undefined>" );
    if( !undefined )
    {
        return false;
    }

    return true;
}

}  // namespace atom
