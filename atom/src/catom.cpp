/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2019, Nucleic Development Team.
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

#include <map>
#include <cppy/cppy.h>
#include "atomref.h"
#include "catom.h"
#include "globalstatic.h"
#include "methodwrapper.h"
#include "packagenaming.h"
#include "utils.h"

#define member_cast( o ) reinterpret_cast<Member*>( o )
#define signal_cast( o ) reinterpret_cast<Signal*>( o )
#define pymethod_cast( o ) reinterpret_cast<PyMethodObject*>( o )


namespace atom
{


namespace
{

static PyObject* atom_members;


PyObject*
CAtom_new( PyTypeObject* type, PyObject* args, PyObject* kwargs )
{
    cppy::ptr membersptr( PyObject_GetAttr( pyobject_cast( type ), atom_members ) );
    if( !membersptr )
        return 0;
    if( !PyDict_CheckExact( membersptr.get() ) )
        return cppy::system_error( "atom members" );
    cppy::ptr selfptr( PyType_GenericNew( type, args, kwargs ) );
    if( !selfptr )
        return 0;
    CAtom* atom = catom_cast( selfptr.get() );
    uint32_t count = static_cast<uint32_t>( PyDict_Size( membersptr.get() ) );
    if( count > 0 )
    {
        if( count > MAX_MEMBER_COUNT )
            return cppy::type_error( "too many members" );
        size_t size = sizeof( PyObject* ) * count;
        void* slots = PyObject_MALLOC( size );
        if( !slots )
            return PyErr_NoMemory();  // LCOV_EXCL_LINE
        memset( slots, 0, size );
        atom->slots = reinterpret_cast<PyObject**>( slots );
        atom->set_slot_count( count );
    }
    atom->set_notifications_enabled( true );
    return selfptr.release();
}


int
CAtom_init( CAtom* self, PyObject* args, PyObject* kwargs )
{
    if( PyTuple_GET_SIZE( args ) > 0 )
    {
        cppy::type_error( "__init__() takes no positional arguments" );
        return -1;
    }
    if( kwargs )
    {
        cppy::ptr selfptr( cppy::incref( pyobject_cast( self ) ) );
        PyObject* key;
        PyObject* value;
        Py_ssize_t pos = 0;
        while( PyDict_Next( kwargs, &pos, &key, &value ) )
        {
            if( !selfptr.setattr( key, value ) )
            {
                return -1;  // LCOV_EXCL_LINE
            }
        }
    }
    return 0;
}


void
CAtom_clear( CAtom* self )
{
    uint32_t count = self->get_slot_count();
    for( uint32_t i = 0; i < count; ++i )
    {
        Py_CLEAR( self->slots[ i ] );
    }
    if( self->observers )
    {
        self->observers->py_clear();
    }
}


int
CAtom_traverse( CAtom* self, visitproc visit, void* arg )
{
    uint32_t count = self->get_slot_count();
    for( uint32_t i = 0; i < count; ++i )
    {
        Py_VISIT( self->slots[ i ] );
    }
#if PY_VERSION_HEX >= 0x03090000
    // This was not needed before Python 3.9 (Python issue 35810 and 40217)
    Py_VISIT(Py_TYPE(self));
#endif
    if( self->observers )
    {
        return self->observers->py_traverse( visit, arg );
    }

    return 0;
}


void
CAtom_dealloc( CAtom* self )
{
    if( self->has_guards() )
    {
        CAtom::clear_guards( self );
    }
    if( self->has_atomref() )
    {
        SharedAtomRef::clear( self );
    }
    PyObject_GC_UnTrack( self );
    CAtom_clear( self );
    if( self->slots )
    {
        PyObject_FREE( self->slots );
    }
    delete self->observers;
    self->observers = 0;
    Py_TYPE(self)->tp_free( pyobject_cast( self ) );
}


PyObject*
CAtom_notifications_enabled( CAtom* self )
{
    return utils::py_bool( self->get_notifications_enabled() );
}


PyObject*
CAtom_set_notifications_enabled( CAtom* self, PyObject* arg )
{
    if( !PyBool_Check( arg ) )
        return cppy::type_error( arg, "bool" );
    bool old = self->get_notifications_enabled();
    self->set_notifications_enabled( arg == Py_True ? true : false );
    return utils::py_bool( old );
}


PyObject*
CAtom_get_member( PyObject* self, PyObject* name )
{
    if( !PyUnicode_Check( name ) )
        return cppy::type_error( name, "str" );
    cppy::ptr membersptr( PyObject_GetAttr( pyobject_cast( Py_TYPE(self) ), atom_members ) );
    if( !membersptr )
        return 0;
    if( !PyDict_CheckExact( membersptr.get() ) )
        return cppy::system_error( "atom members" );
    cppy::ptr member( cppy::xincref( PyDict_GetItem( membersptr.get(), name ) ) );
    if( !member )
        Py_RETURN_NONE;
    return member.release();
}


PyObject*
CAtom_observe( CAtom* self, PyObject* args )
{
    if( PyTuple_GET_SIZE( args ) != 2 )
        return cppy::type_error( "observe() takes exactly 2 arguments" );
    PyObject* topic = PyTuple_GET_ITEM( args, 0 );
    PyObject* callback = PyTuple_GET_ITEM( args, 1 );
    if( !PyCallable_Check( callback ) )
        return cppy::type_error( callback, "callable" );
    if( utils::str_check( topic ) )
    {
        if( !self->observe( topic, callback ) )
            return 0;
    }
    else
    {
        cppy::ptr iterator( PyObject_GetIter( topic ) );
        if( !iterator )
            return 0;
        cppy::ptr topicptr;
        while( ( topicptr = PyIter_Next( iterator.get() ) ) )
        {
            if( !utils::str_check( topicptr.get() ) )
                return cppy::type_error( topicptr.get(), "str" );
            if( !self->observe( topicptr.get(), callback ) )
                return 0;
        }
        if( PyErr_Occurred() )
            return 0;
    }
    Py_RETURN_NONE;
}


PyObject*
_CAtom_unobserve_0( CAtom* self )
{
    if( !self->unobserve() )
        return 0;
    Py_RETURN_NONE;
}


PyObject*
_CAtom_unobserve_1( CAtom* self, PyObject* topic )
{
    if( utils::str_check( topic ) )
    {
        if( !self->unobserve( topic ) )
            return 0;
    }
    else
    {
        cppy::ptr iterator( PyObject_GetIter( topic ) );
        if( !iterator )
            return 0;
        cppy::ptr topicptr;
        while( ( topicptr = PyIter_Next( iterator.get() ) ) )
        {
            if( !utils::str_check( topicptr.get() ) )
                return cppy::type_error( topicptr.get(), "str" );
            if( !self->unobserve( topicptr.get() ) )
                return 0;
        }
        if( PyErr_Occurred() )
            return 0;
    }
    Py_RETURN_NONE;
}


PyObject*
_CAtom_unobserve_2( CAtom* self, PyObject* topic, PyObject* callback )
{
    if( !PyCallable_Check( callback ) )
        return cppy::type_error( callback, "callable" );
    if( utils::str_check( topic ) )
    {
        if( !self->unobserve( topic, callback ) )
            return 0;
    }
    else
    {
        cppy::ptr iterator( PyObject_GetIter( topic ) );
        if( !iterator )
            return 0;
        cppy::ptr topicptr;
        while( ( topicptr = PyIter_Next( iterator.get() ) ) )
        {
            if( !utils::str_check( topicptr.get() ) )
                return cppy::type_error( topicptr.get(), "str" );
            if( !self->unobserve( topicptr.get(), callback ) )
                return 0;
        }
        if( PyErr_Occurred() )
            return 0;
    }
    Py_RETURN_NONE;
}


PyObject*
CAtom_unobserve( CAtom* self, PyObject* args )
{
    Py_ssize_t n_args = PyTuple_GET_SIZE( args );
    if( n_args > 2 )
        return cppy::type_error( "unobserve() takes at most 2 arguments" );
    if( n_args == 0 )
        return _CAtom_unobserve_0( self );
    if( n_args == 1 )
        return _CAtom_unobserve_1( self, PyTuple_GET_ITEM( args, 0 ) );
    return _CAtom_unobserve_2( self, PyTuple_GET_ITEM( args, 0 ),
        PyTuple_GET_ITEM( args, 1 ) );
}


PyObject*
CAtom_has_observers( CAtom* self, PyObject* topic )
{
    return utils::py_bool( self->has_observers( topic ) );
}


PyObject*
CAtom_has_observer( CAtom* self, PyObject* args )
{
    if( PyTuple_GET_SIZE( args ) != 2 )
        return cppy::type_error( "has_observer() takes exactly 2 arguments" );
    PyObject* topic = PyTuple_GET_ITEM( args, 0 );
    PyObject* callback = PyTuple_GET_ITEM( args, 1 );
    if( !utils::str_check( topic ) )
        return cppy::type_error( topic, "str" );
    if( !PyCallable_Check( callback ) )
        return cppy::type_error( callback, "callable" );
    return utils::py_bool( self->has_observer( topic, callback ) );
}


PyObject*
CAtom_notify( CAtom* self, PyObject* args, PyObject* kwargs )
{
    if( PyTuple_GET_SIZE( args ) < 1 )
        return cppy::type_error( "notify() requires at least 1 argument" );
    PyObject* topic = PyTuple_GET_ITEM( args, 0 );
    if( !utils::str_check( topic ) )
        return cppy::type_error( topic, "str" );
    cppy::ptr argsptr( PyTuple_GetSlice( args, 1, PyTuple_GET_SIZE( args ) ) );
    if( !argsptr )
        return 0;
    if( !self->notify( topic, argsptr.get(), kwargs ) )
        return 0;
    Py_RETURN_NONE;
}


PyObject*
CAtom_freeze( CAtom* self )
{
    self->set_frozen( true );
    Py_RETURN_NONE;
}


PyObject*
CAtom_sizeof( CAtom* self, PyObject* args )
{
    Py_ssize_t size = Py_TYPE(self)->tp_basicsize;
    size += sizeof( PyObject* ) * self->get_slot_count();
    if( self->observers )
        size += self->observers->py_sizeof();
    return PyLong_FromSsize_t( size );
}


static PyMethodDef
CAtom_methods[] = {
    { "notifications_enabled", ( PyCFunction )CAtom_notifications_enabled, METH_NOARGS,
      "Get whether notification is enabled for the atom." },
    { "set_notifications_enabled", ( PyCFunction )CAtom_set_notifications_enabled, METH_O,
      "Enable or disable notifications for the atom." },
    { "get_member", ( PyCFunction )CAtom_get_member, METH_O,
      "Get the named member for the atom." },
    { "observe", ( PyCFunction )CAtom_observe, METH_VARARGS,
      "Register an observer callback to observe changes on the given topic(s)." },
    { "unobserve", ( PyCFunction )CAtom_unobserve, METH_VARARGS,
      "Unregister an observer callback for the given topic(s)." },
    { "has_observers", ( PyCFunction )CAtom_has_observers, METH_O,
      "Get whether the atom has observers for a given topic." },
    { "has_observer", ( PyCFunction )CAtom_has_observer, METH_VARARGS,
      "Get whether the atom has the given observer for a given topic." },
    { "notify", ( PyCFunction )CAtom_notify, METH_VARARGS | METH_KEYWORDS,
      "Call the registered observers for a given topic with positional and keyword arguments." },
    { "freeze", ( PyCFunction )CAtom_freeze, METH_NOARGS,
      "Freeze the atom to prevent further modifications to its attributes." },
    { "__sizeof__", ( PyCFunction )CAtom_sizeof, METH_NOARGS,
      "__sizeof__() -> size of object in memory, in bytes" },
    { 0 } // sentinel
};


static PyType_Slot Atom_Type_slots[] = {
    { Py_tp_dealloc, void_cast( CAtom_dealloc ) },              /* tp_dealloc */
    { Py_tp_traverse, void_cast( CAtom_traverse ) },            /* tp_traverse */
    { Py_tp_clear, void_cast( CAtom_clear ) },                  /* tp_clear */
    { Py_tp_methods, void_cast( CAtom_methods ) },              /* tp_methods */
    { Py_tp_new, void_cast( CAtom_new ) },                      /* tp_new */
    { Py_tp_init, void_cast( CAtom_init) },                     /* tp_new */
    { Py_tp_alloc, void_cast( PyType_GenericAlloc ) },          /* tp_alloc */
    { Py_tp_free, void_cast( PyObject_GC_Del ) },               /* tp_free */
    { 0, 0 },
};


}  // namespace


// Initialize static variables (otherwise the compiler eliminates them)
PyTypeObject* CAtom::TypeObject = NULL;


PyType_Spec CAtom::TypeObject_Spec = {
	PACKAGE_TYPENAME( "CAtom" ),               /* tp_name */
	sizeof( CAtom ),                           /* tp_basicsize */
	0,                                          /* tp_itemsize */
	Py_TPFLAGS_DEFAULT
    |Py_TPFLAGS_BASETYPE
    |Py_TPFLAGS_HAVE_GC,                        /* tp_flags */
    Atom_Type_slots                         /* slots */
};


bool CAtom::Ready()
{

    if( !MethodWrapper::Ready() )
    {
        return false;
    }
    if( !AtomMethodWrapper::Ready() )
    {
        return false;
    }

    // The reference will be handled by the module to which we will add the type
	TypeObject = pytype_cast( PyType_FromSpec( &TypeObject_Spec ) );
    if( !TypeObject )
    {
        return false;
    }
    atom_members = PyUnicode_FromString( "__atom_members__" );
    if( !atom_members )
    {
        return false;
    }

    return true;
}


static PyObject*
wrap_callback( PyObject* callback )
{
    if( PyMethod_Check( callback ) && PyMethod_GET_SELF( callback ) )
        return MethodWrapper::New( callback );
    return cppy::incref( callback );
}


bool
CAtom::observe( PyObject* topic, PyObject* callback )
{
    cppy::ptr topicptr( cppy::incref( topic ) );
    cppy::ptr callbackptr( wrap_callback( callback ) );
    if( !callbackptr )
        return false;
    if( !observers )
        observers = new ObserverPool();
    observers->add( topicptr, callbackptr );
    return true;
}


bool
CAtom::unobserve( PyObject* topic, PyObject* callback )
{
    if( !observers )
        return true;
    cppy::ptr topicptr( cppy::incref( topic ) );
    cppy::ptr callbackptr( cppy::incref( callback ) );
    observers->remove( topicptr, callbackptr );
    return true;
}


bool
CAtom::unobserve( PyObject* topic )
{
    if( !observers )
        return true;
    cppy::ptr topicptr( cppy::incref( topic ) );
    observers->remove( topicptr );
    return true;
}


bool
CAtom::unobserve()
{
    if( !observers )
        return true;
    observers->py_clear();
    return true;
}


bool
CAtom::notify( PyObject* topic, PyObject* args, PyObject* kwargs )
{
    if( observers && get_notifications_enabled() )
    {
        cppy::ptr topicptr( cppy::incref( topic ) );
        cppy::ptr argsptr( cppy::incref( args ) );
        cppy::ptr kwargsptr( cppy::xincref( kwargs ) );
        if( !observers->notify( topicptr, argsptr, kwargsptr ) )
            return false;
    }
    return true;
}


// shamelessly derived from qobject.h
typedef std::multimap<CAtom*, CAtom**> GuardMap;
GLOBAL_STATIC( GuardMap, guard_map )


void CAtom::add_guard( CAtom** ptr )
{
    if( !*ptr )
        return;
    GuardMap* map = guard_map();
    if( !map )
    {
        *ptr = 0;  // LCOV_EXCL_LINE
        return;  // LCOV_EXCL_LINE
    }
    map->insert( GuardMap::value_type( *ptr, ptr ) );
    ( *ptr )->set_has_guards( true );
}


void CAtom::remove_guard( CAtom** ptr )
{
    if( !*ptr )
        return;
    GuardMap* map = guard_map();
    if( !map || map->empty() )
        return;
    bool more = false;  // if the CAtom has more pointers attached to it.
    GuardMap::iterator it = map->find( *ptr );
    const GuardMap::iterator end = map->end();
    for( ; it != end && it->first == *ptr; ++it )
    {
        if( it->second == ptr )
        {
            if( !more )
            {
                ++it;
                more = ( it != end ) && ( it->first == *ptr );
                --it;
            }
            map->erase( it );
            break;
        }
        more = true;
    }
    if( !more )
        ( *ptr )->set_has_guards( false );
}


void CAtom::change_guard( CAtom** ptr, CAtom* o )
{
    GuardMap* map = guard_map();
    if( !map )
    {
        *ptr = 0;  // LCOV_EXCL_LINE
        return;  // LCOV_EXCL_LINE
    }
    if( o )
    {
        map->insert( GuardMap::value_type( o, ptr ) );
        o->set_has_guards( true );
    }
    CAtom::remove_guard( ptr );
    *ptr = o;
}


void CAtom::clear_guards( CAtom* o )
{
    GuardMap* map = 0;
    try
    {
        map = guard_map();
    }
    catch( std::bad_alloc& )
    {
        // do nothing in case of OOM - code below is safe
    }
    if( !map || map->empty() )
        return;
    GuardMap::iterator it = map->find( o );
    GuardMap::iterator first = it;
    const GuardMap::iterator end = map->end();
    for( ; it != end && it->first == o; ++it )
        *it->second = 0;
    map->erase( first, it );
    o->set_has_guards( false );
}


}  // namespace atom
