/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2019, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file LICENSE, distributed with this software.
|----------------------------------------------------------------------------*/
#include <cppy/cppy.h>
#include <algorithm>
#include <vector>
#include <iostream>
#include <sstream>
#include "packagenaming.h"
#include "utils.h"

#ifdef __clang__
#pragma clang diagnostic ignored "-Wdeprecated-writable-strings"
#endif

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif


namespace
{

class MapItem
{

public:

    MapItem() {}

    MapItem( PyObject* key, PyObject* value ) :
        m_key( cppy::incref( key ) ), m_value( cppy::incref( value ) ) { }

    MapItem( cppy::ptr& key, cppy::ptr& value ) :
        m_key( key ), m_value( value ) { }

    MapItem( cppy::ptr& key, PyObject* value ) :
        m_key( key ), m_value( cppy::incref( value ) ) { }

    MapItem( PyObject* key, cppy::ptr& value ) :
        m_key( cppy::incref( key ) ), m_value( value ) { }

    ~MapItem() { }

    PyObject* key()
    {
        return m_key.get();
    }

    PyObject* value()
    {
        return m_value.get();
    }

    void update( PyObject* value )
    {
        m_value = cppy::incref( value );
    }

    struct CmpLess
    {
        // All three operators are needed in order to keep the
        // MSVC debug version of std::lower_bound happy.
        bool operator()( MapItem& first, MapItem& second )
        {
            if( first.m_key == second.m_key )
                return false;
            return atom::utils::safe_richcompare( first.m_key.get(), second.m_key.get(), Py_LT );
        }

        bool operator()( MapItem& first, PyObject* second )
        {
            if( first.m_key == second )
                return false;
            return atom::utils::safe_richcompare( first.m_key.get(), second, Py_LT );
        }

        bool operator()( PyObject* first, MapItem& second )
        {
            if( first == second.m_key )
                return false;
            return atom::utils::safe_richcompare( first, second.m_key.get(), Py_LT );
        }
    };

    struct CmpEq
    {
        bool operator()( MapItem& first, MapItem& second )
        {
            if( first.m_key == second.m_key )
                return true;
            return atom::utils::safe_richcompare( first.m_key.get(), second.m_key.get(), Py_EQ );
        }

        bool operator()( MapItem& first, PyObject* second )
        {
            if( first.m_key == second )
                return true;
            return atom::utils::safe_richcompare( first.m_key.get(),  second, Py_EQ );
        }

        bool operator()( PyObject* first, MapItem& second )
        {
            if( first == second.m_key )
                return true;
            return atom::utils::safe_richcompare( first, second.m_key.get(), Py_EQ );
        }
    };

private:

    cppy::ptr m_key;
    cppy::ptr m_value;
};


struct SortedMap
{
    typedef std::vector<MapItem> Items;

    PyObject_HEAD
    Items* m_items;

    static PyType_Spec TypeObject_Spec;

    static PyTypeObject* TypeObject;

	static bool Ready();

    PyObject* getitem( PyObject* key, PyObject* default_value = 0 )
    {
        Items::iterator it = std::lower_bound(
            m_items->begin(), m_items->end(), key, MapItem::CmpLess()
        );
        if( it == m_items->end() )
        {
            if( default_value )
                return cppy::incref( default_value );
            return lookup_fail( key );
        }
        if( MapItem::CmpEq()( *it, key ) )
            return cppy::incref( it->value() );
        if( default_value )
            return cppy::incref( default_value );
        return lookup_fail( key );
    }

    int setitem( PyObject* key, PyObject* value )
    {
        Items::iterator it = std::lower_bound(
            m_items->begin(), m_items->end(), key, MapItem::CmpLess()
        );
        if( it == m_items->end() )
            m_items->push_back( MapItem( key, value ) );
        else if( MapItem::CmpEq()( *it, key ) )
            it->update( value );
        else
            m_items->insert( it, MapItem( key, value ) );
        return 0;
    }

    int delitem( PyObject* key )
    {
        Items::iterator it = std::lower_bound(
            m_items->begin(), m_items->end(), key, MapItem::CmpLess()
        );
        if( it == m_items->end() )
        {
            lookup_fail( key );
            return -1;
        }
        if( MapItem::CmpEq()( *it, key ) )
        {
            m_items->erase( it );
            return 0;
        }
        lookup_fail( key );
        return -1;
    }

    bool contains( PyObject* key )
    {
        Items::iterator it = std::lower_bound(
            m_items->begin(), m_items->end(), key, MapItem::CmpLess()
        );
        if( it == m_items->end() )
            return false;
        return MapItem::CmpEq()( *it, key );
    }

    PyObject* pop( PyObject* key, PyObject* default_value=0 )
    {
        Items::iterator it = std::lower_bound(
            m_items->begin(), m_items->end(), key, MapItem::CmpLess()
        );
        if( it == m_items->end() )
        {
            if( default_value )
                return cppy::incref( default_value );
            return lookup_fail( key );
        }
        if( MapItem::CmpEq()( *it, key ) )
        {
            PyObject* res = cppy::incref( it->value() );
            m_items->erase( it );
            return res;
        }
        if( default_value )
            return cppy::incref( default_value );
        return lookup_fail( key );
    }

    PyObject* keys()
    {
        PyObject* pylist = PyList_New( m_items->size() );
        if( !pylist )
            return 0;
        Py_ssize_t listidx = 0;
        Items::iterator it;
        Items::iterator end_it = m_items->end();
        for( it = m_items->begin(); it != end_it; ++it )
        {
            PyList_SET_ITEM( pylist, listidx, cppy::incref( it->key() ) );
            ++listidx;
        }
        return pylist;
    }

    PyObject* values()
    {
        PyObject* pylist = PyList_New( m_items->size() );
        if( !pylist )
            return 0;
        Py_ssize_t listidx = 0;
        Items::iterator it;
        Items::iterator end_it = m_items->end();
        for( it = m_items->begin(); it != end_it; ++it )
        {
            PyList_SET_ITEM( pylist, listidx, cppy::incref( it->value() ) );
            ++listidx;
        }
        return pylist;
    }

    PyObject* items()
    {
        PyObject* pylist = PyList_New( m_items->size() );
        if( !pylist )
            return 0;
        Py_ssize_t listidx = 0;
        Items::iterator it;
        Items::iterator end_it = m_items->end();
        for( it = m_items->begin(); it != end_it; ++it )
        {
            PyObject* pytuple = PyTuple_New( 2 );
            if( !pytuple )
                return 0;
            PyTuple_SET_ITEM( pytuple, 0, cppy::incref( it->key() ) );
            PyTuple_SET_ITEM( pytuple, 1, cppy::incref( it->value() ) );
            PyList_SET_ITEM( pylist, listidx, pytuple );
            ++listidx;
        }
        return pylist;
    }

    static PyObject* lookup_fail( PyObject* key )
    {
        cppy::ptr pystr( PyObject_Str( key ) );
        if( !pystr )
            return 0;
        cppy::ptr pytuple( PyTuple_Pack( 1, key ) );
        if (!pytuple)
            return 0;
        PyErr_SetObject(PyExc_KeyError, pytuple.get());
        return 0;
    }
};


PyObject*
SortedMap_new( PyTypeObject* type, PyObject* args, PyObject* kwargs )
{
    PyObject* map = 0;
    static char* kwlist[] = { "map", 0 };
    if( !PyArg_ParseTupleAndKeywords( args, kwargs, "|O:__new__", kwlist, &map ) )
        return 0;

    PyObject* self = PyType_GenericNew( type, 0, 0 );
    if( !self )
        return 0;
    SortedMap* cself = reinterpret_cast<SortedMap*>( self );
    cself->m_items = new SortedMap::Items();

    cppy::ptr seq;
    if( map )
    {
        if( PyDict_Check( map ) )
        {
            seq = PyObject_GetIter( PyDict_Items( map ) );
            if( !seq )
                return 0;
        }
        else
        {
            seq = PyObject_GetIter( map );
            if( !seq )
                return 0;
        }
    }

    if( seq )
    {
        cppy::ptr item;
        while( (item = PyIter_Next( seq.get() )) )
        {
            if( PySequence_Length( item.get() ) != 2)
                return cppy::type_error( item.get(), "pairs of objects" );

            cself->setitem( PySequence_GetItem( item.get(), 0 ),
                            PySequence_GetItem( item.get(), 1 ) );
        }
    }

    return self;
}

// Clearing the vector may cause arbitrary side effects on item
// decref, including calls into methods which mutate the vector.
// To avoid segfaults, first make the vector empty, then let the
// destructors run for the old items.
int
SortedMap_clear( SortedMap* self )
{
    SortedMap::Items empty;
    self->m_items->swap( empty );
    return 0;
}


int
SortedMap_traverse( SortedMap* self, visitproc visit, void* arg )
{
    SortedMap::Items::iterator it;
    SortedMap::Items::iterator end_it = self->m_items->end();
    for( it = self->m_items->begin(); it != end_it; ++it )
    {
        Py_VISIT( it->key() );
        Py_VISIT( it->value() );
    }
#if PY_VERSION_HEX >= 0x03090000
    // This was not needed before Python 3.9 (Python issue 35810 and 40217)
    Py_VISIT(Py_TYPE(self));
#endif
    return 0;
}


void
SortedMap_dealloc( SortedMap* self )
{
    PyObject_GC_UnTrack( self );
    SortedMap_clear( self );
    delete self->m_items;
    self->m_items = 0;
    Py_TYPE(self)->tp_free( reinterpret_cast<PyObject*>( self ) );
}


Py_ssize_t
SortedMap_length( SortedMap* self )
{
    return static_cast<Py_ssize_t>( self->m_items->size() );
}


PyObject*
SortedMap_subscript( SortedMap* self, PyObject* key )
{
    return self->getitem( key );
}


int
SortedMap_ass_subscript( SortedMap* self, PyObject* key, PyObject* value )
{
    if( !value )
        return self->delitem( key );
    return self->setitem( key, value );
}


int
SortedMap_contains( SortedMap* self, PyObject* key )
{
    return self->contains( key );
}


PyObject*
SortedMap_get( SortedMap* self, PyObject* args )
{
    Py_ssize_t nargs = PyTuple_GET_SIZE( args );
    if( nargs == 1 )
        return self->getitem( PyTuple_GET_ITEM( args, 0 ), Py_None );
    if( nargs == 2 )
        return self->getitem( PyTuple_GET_ITEM( args, 0 ), PyTuple_GET_ITEM( args, 1 ) );
    std::ostringstream ostr;
    if( nargs > 2 )
        ostr << "get() expected at most 2 arguments, got " << nargs;
    else
        ostr << "get() expected at least 1 argument, got " << nargs;
    return cppy::type_error( ostr.str().c_str() );
}


PyObject*
SortedMap_pop( SortedMap* self, PyObject* args )
{
    Py_ssize_t nargs = PyTuple_GET_SIZE( args );
    if( nargs == 1 )
        return self->pop( PyTuple_GET_ITEM( args, 0 ) );
    if( nargs == 2 )
        return self->getitem( PyTuple_GET_ITEM( args, 0 ), PyTuple_GET_ITEM( args, 1 ) );
    std::ostringstream ostr;
    if( nargs > 2 )
        ostr << "pop() expected at most 2 arguments, got " << nargs;
    else
        ostr << "pop() expected at least 1 argument, got " << nargs;
    return cppy::type_error( ostr.str().c_str() );
}


PyObject*
SortedMap_clearmethod( SortedMap* self )
{
    // Clearing the vector may cause arbitrary side effects on item
    // decref, including calls into methods which mutate the vector.
    // To avoid segfaults, first make the vector empty, then let the
    // destructors run for the old items.
    SortedMap::Items empty;
    self->m_items->swap( empty );
    Py_RETURN_NONE;
}


PyObject*
SortedMap_keys( SortedMap* self )
{
    return self->keys();
}


PyObject*
SortedMap_values( SortedMap* self )
{
    return self->values();
}


PyObject*
SortedMap_items( SortedMap* self )
{
    return self->items();
}


PyObject*
SortedMap_iter( SortedMap* self )
{
    cppy::ptr keys( self->keys() );
    if( !keys )
        return 0;
    return PyObject_GetIter( keys.get() );
}



PyObject*
SortedMap_copy( SortedMap* self )
{
    PyTypeObject* type = pytype_cast( Py_TYPE(self) );
    PyObject* copy = type->tp_alloc( type, 0 );
    if( !copy )
        return 0;
    SortedMap* ccopy = reinterpret_cast<SortedMap*>( copy );
    ccopy->m_items = new SortedMap::Items();
    *ccopy->m_items = *self->m_items;
    return copy;
}


PyObject*
SortedMap_repr( SortedMap* self )
{
    std::ostringstream ostr;
    ostr << "sortedmap([";
    SortedMap::Items::iterator it;
    SortedMap::Items::iterator end_it = self->m_items->end();
    for( it = self->m_items->begin(); it != end_it; ++it )
    {
        cppy::ptr keystr( PyObject_Repr( it->key() ) );
        if( !keystr )
            return 0;
        cppy::ptr valstr( PyObject_Repr( it->value() ) );
        if( !valstr )
            return 0;
        ostr << "(" << PyUnicode_AsUTF8( keystr.get() ) << ", ";
        ostr << PyUnicode_AsUTF8( valstr.get() ) << "), ";
    }
    if( self->m_items->size() > 0 )
        ostr.seekp( -2, std::ios_base::cur );
    ostr << "])";
    return PyUnicode_FromString( ostr.str().c_str() );
}


PyObject*
SortedMap_contains_bool( SortedMap* self, PyObject* key )
{
    if( self->contains( key ) )
    {
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}


PyObject*
SortedMap_sizeof( SortedMap* self, PyObject* args )
{
    Py_ssize_t size = Py_TYPE(self)->tp_basicsize;
    size += sizeof( SortedMap::Items );
    size += sizeof( MapItem ) * self->m_items->capacity();
    return PyLong_FromSsize_t( size );
}


static PyMethodDef
SortedMap_methods[] = {
    { "get", ( PyCFunction )SortedMap_get, METH_VARARGS,
      "" },
    { "pop", ( PyCFunction )SortedMap_pop, METH_VARARGS,
      "" },
    { "clear", ( PyCFunction)SortedMap_clearmethod, METH_NOARGS,
      "" },
    { "keys", ( PyCFunction )SortedMap_keys, METH_NOARGS,
      "" },
    { "values", ( PyCFunction )SortedMap_values, METH_NOARGS,
      "" },
    { "items", ( PyCFunction )SortedMap_items, METH_NOARGS,
      "" },
    { "copy", ( PyCFunction )SortedMap_copy, METH_NOARGS,
      "" },
    { "__contains__", ( PyCFunction )SortedMap_contains_bool, METH_O | METH_COEXIST,
      "" },
    { "__getitem__", ( PyCFunction )SortedMap_subscript, METH_O | METH_COEXIST,
      "" },
    { "__sizeof__", ( PyCFunction )SortedMap_sizeof, METH_NOARGS,
      "__sizeof__() -> size of object in memory, in bytes" },
    { 0 } // sentinel
};


static PyType_Slot SortedMap_Type_slots[] = {
    { Py_tp_dealloc, void_cast( SortedMap_dealloc ) },              /* tp_dealloc */
    { Py_tp_traverse, void_cast( SortedMap_traverse ) },            /* tp_traverse */
    { Py_tp_clear, void_cast( SortedMap_clear ) },                  /* tp_clear */
    { Py_tp_methods, void_cast( SortedMap_methods ) },              /* tp_methods */
    { Py_tp_repr, void_cast( SortedMap_repr ) },                    /* tp_repr */
    { Py_tp_new, void_cast( SortedMap_new ) },                      /* tp_new */
    { Py_tp_iter, void_cast( SortedMap_iter ) },                    /* tp_iter */
    { Py_tp_alloc, void_cast( PyType_GenericAlloc ) },              /* tp_alloc */
    { Py_mp_length, void_cast( SortedMap_length ) },                /* mp_length */
    { Py_mp_subscript, void_cast( SortedMap_subscript ) },          /* mp_subscript */
    { Py_mp_ass_subscript, void_cast( SortedMap_ass_subscript ) },  /* mp_ass_subscript */
    { Py_sq_contains, void_cast( SortedMap_contains ) },            /* sq_contains */
    { 0, 0 },
};


// Initialize static variables (otherwise the compiler eliminates them)
PyTypeObject* SortedMap::TypeObject = NULL;


PyType_Spec SortedMap::TypeObject_Spec = {
	PACKAGE_TYPENAME( "sortedmap.sortedmap" ),   /* tp_name */
	sizeof( SortedMap ),                         /* tp_basicsize */
	0,                                           /* tp_itemsize */
	Py_TPFLAGS_DEFAULT|
    Py_TPFLAGS_HAVE_GC,                          /* tp_flags */
    SortedMap_Type_slots                         /* slots */
};


bool SortedMap::Ready()
{
    // The reference will be handled by the module to which we will add the type
	TypeObject = pytype_cast( PyType_FromSpec( &TypeObject_Spec ) );
    if( !TypeObject )
    {
        return false;
    }
    return true;
}


// Module creation

static PyMethodDef
sortedmap_methods[] = {
    { 0 } // Sentinel
};

int
sortedmap_modexec( PyObject *mod )
{
    if( !SortedMap::Ready() )
    {
        return -1;
    }

    cppy::ptr sortedmap( pyobject_cast( SortedMap::TypeObject ) );
	if( PyModule_AddObject( mod, "sortedmap", sortedmap.get() ) < 0 )
	{
		return false;
	}
    sortedmap.release();


    return 0;
}


PyModuleDef_Slot sortedmap_slots[] = {
    {Py_mod_exec, reinterpret_cast<void*>( sortedmap_modexec ) },
    {0, NULL}
};


static struct PyModuleDef moduledef = {
        PyModuleDef_HEAD_INIT,
        "sortedmap",
        "sortedmap extension module",
        0,
        sortedmap_methods,
        sortedmap_slots,
        NULL,
        NULL,
        NULL
};


}  // namespace


PyMODINIT_FUNC PyInit_sortedmap( void )
{
    return PyModuleDef_Init( &moduledef );
}
