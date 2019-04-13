/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2017, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include <cppy/cppy.h>
#include <algorithm>
#include <vector>
#include <iostream>
#include <sstream>

#ifdef __clang__
#pragma clang diagnostic ignored "-Wdeprecated-writable-strings"
#endif

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif


/**
 * A fallback 3way comparison function for when PyObject_RichCompareBool
 * fails to compare "unorderable types" on Python 3.
 *
 * This is based on Python 2's `default_3way_compare`.
 *
 * This function will not change the Python exception state.
 */
inline int
fallback_3way_compare( PyObject* first, PyObject* second )
{
    // Compare pointer values if the types are the same.
    if( first->ob_type == second->ob_type ) {
        Py_uintptr_t fp = reinterpret_cast<Py_uintptr_t>( first );
        Py_uintptr_t sp = reinterpret_cast<Py_uintptr_t>( second );
        return (fp < sp) ? -1 : (fp > sp) ? 1 : 0;
    }

    // None is smaller than anything.
    if( first == Py_None )
        return -1;
    if( second == Py_None )
        return 1;

    // Compare based on type names, numbers are smaller.
    const char* fn = PyNumber_Check( first ) ? "" : first->ob_type->tp_name;
    const char* sn = PyNumber_Check( second ) ? "" : second->ob_type->tp_name;
    int c = strcmp( fn, sn );
    if( c < 0 )
        return -1;
    if( c > 0 )
        return 1;

    // Finally, fall back to comparing type pointers.
    Py_uintptr_t ftp = reinterpret_cast<Py_uintptr_t>( first->ob_type );
    Py_uintptr_t stp = reinterpret_cast<Py_uintptr_t>( second->ob_type );
    return ftp < stp ? -1 : 1;
}


bool richcompare( PyObject* first, PyObject* second, int opid )
{
    // Start with Python's rich compare.
    int r = PyObject_RichCompareBool( first, second, opid );

    // Handle a successful comparison.
    if( r == 1 )
        return true;
    if( r == 0 )
        return false;

    // Clear the error if one happened because we attempted an invalid
    // comparison.
    if( PyErr_Occurred() )
        PyErr_Clear();

    // Fallback to the Python 2 default 3 way compare.
    int c = fallback_3way_compare( first, second );

    // Convert the 3way comparison result based on the `opid`.
    switch (opid) {
    case Py_EQ: return c == 0;
    case Py_NE: return c != 0;
    case Py_LE: return c <= 0;
    case Py_GE: return c >= 0;
    case Py_LT: return c < 0;
    case Py_GT: return c > 0;
    }

    // Return `false` if the `opid` is not handled.
    return false;
}


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
            return richcompare( first.m_key.get(), second.m_key.get(), Py_LT );
        }

        bool operator()( MapItem& first, PyObject* second )
        {
            if( first.m_key == second )
                return false;
            return richcompare( first.m_key.get(), second, Py_LT );
        }

        bool operator()( PyObject* first, MapItem& second )
        {
            if( first == second.m_key )
                return false;
            return richcompare( first, second.m_key.get(), Py_LT );
        }
    };

    struct CmpEq
    {
        bool operator()( MapItem& first, MapItem& second )
        {
            if( first.m_key == second.m_key )
                return true;
            return richcompare( first.m_key.get(), second.m_key.get(), Py_EQ );
        }

        bool operator()( MapItem& first, PyObject* second )
        {
            if( first.m_key == second )
                return true;
            return richcompare( first.m_key.get(),  second, Py_EQ );
        }

        bool operator()( PyObject* first, MapItem& second )
        {
            if( first == second.m_key )
                return true;
            return richcompare( first, second.m_key.get(), Py_EQ );
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


static PyObject*
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
static int
SortedMap_clear( SortedMap* self )
{
    SortedMap::Items empty;
    self->m_items->swap( empty );
    return 0;
}


static int
SortedMap_traverse( SortedMap* self, visitproc visit, void* arg )
{
    SortedMap::Items::iterator it;
    SortedMap::Items::iterator end_it = self->m_items->end();
    for( it = self->m_items->begin(); it != end_it; ++it )
    {
        Py_VISIT( it->key() );
        Py_VISIT( it->value() );
    }
    return 0;
}


static void
SortedMap_dealloc( SortedMap* self )
{
    PyObject_GC_UnTrack( self );
    SortedMap_clear( self );
    delete self->m_items;
    self->m_items = 0;
    Py_TYPE(self)->tp_free( reinterpret_cast<PyObject*>( self ) );
}


static Py_ssize_t
SortedMap_length( SortedMap* self )
{
    return static_cast<Py_ssize_t>( self->m_items->size() );
}


static PyObject*
SortedMap_subscript( SortedMap* self, PyObject* key )
{
    return self->getitem( key );
}


static int
SortedMap_ass_subscript( SortedMap* self, PyObject* key, PyObject* value )
{
    if( !value )
        return self->delitem( key );
    return self->setitem( key, value );
}


static PyMappingMethods
SortedMap_as_mapping = {
    ( lenfunc )SortedMap_length,              /*mp_length*/
    ( binaryfunc )SortedMap_subscript,        /*mp_subscript*/
    ( objobjargproc )SortedMap_ass_subscript, /*mp_ass_subscript*/
};


static int
SortedMap_contains( SortedMap* self, PyObject* key )
{
    return self->contains( key );
}


static PySequenceMethods
SortedMap_as_sequence = {
    0,                                /* sq_length */
    0,                                /* sq_concat */
    0,                                /* sq_repeat */
    0,                                /* sq_item */
    0,                                /* sq_slice */
    0,                                /* sq_ass_item */
    0,                                /* sq_ass_slice */
    ( objobjproc )SortedMap_contains, /* sq_contains */
    0,                                /* sq_inplace_concat */
    0,                                /* sq_inplace_repeat */
};


static PyObject*
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


static PyObject*
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


static PyObject*
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


static PyObject*
SortedMap_keys( SortedMap* self )
{
    return self->keys();
}


static PyObject*
SortedMap_values( SortedMap* self )
{
    return self->values();
}


static PyObject*
SortedMap_items( SortedMap* self )
{
    return self->items();
}


static PyObject*
SortedMap_iter( SortedMap* self )
{
    cppy::ptr keys( self->keys() );
    if( !keys )
        return 0;
    return PyObject_GetIter( keys.get() );
}



static PyObject*
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


static PyObject*
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


static PyObject*
SortedMap_contains_bool( SortedMap* self, PyObject* key )
{
    if( self->contains( key ) )
        Py_RETURN_TRUE;
    Py_RETURN_FALSE;
}


static PyObject*
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


PyTypeObject SortedMap_Type = {
    PyVarObject_HEAD_INIT( NULL, 0 )
    "atom.sortedmap.sortedmap",                   /* tp_name */
    sizeof( SortedMap ),                          /* tp_basicsize */
    0,                                            /* tp_itemsize */
    ( destructor )SortedMap_dealloc,              /* tp_dealloc */
    ( printfunc )0,                               /* tp_print */
    ( getattrfunc )0,                             /* tp_getattr */
    ( setattrfunc )0,                             /* tp_setattr */
	( PyAsyncMethods* )0,                         /* tp_as_async */
    ( reprfunc )SortedMap_repr,                   /* tp_repr */
    ( PyNumberMethods* )0,                        /* tp_as_number */
    ( PySequenceMethods* )&SortedMap_as_sequence, /* tp_as_sequence */
    ( PyMappingMethods* )&SortedMap_as_mapping,   /* tp_as_mapping */
    ( hashfunc )0,                                /* tp_hash */
    ( ternaryfunc )0,                             /* tp_call */
    ( reprfunc )0,                                /* tp_str */
    ( getattrofunc )0,                            /* tp_getattro */
    ( setattrofunc )0,                            /* tp_setattro */
    ( PyBufferProcs* )0,                          /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC,      /* tp_flags */
    0,                                            /* Documentation string */
    ( traverseproc )SortedMap_traverse,           /* tp_traverse */
    ( inquiry )SortedMap_clear,                   /* tp_clear */
    ( richcmpfunc )0,                             /* tp_richcompare */
    0,                                            /* tp_weaklistoffset */
    ( getiterfunc )SortedMap_iter,                /* tp_iter */
    ( iternextfunc )0,                            /* tp_iternext */
    ( struct PyMethodDef* )SortedMap_methods,     /* tp_methods */
    ( struct PyMemberDef* )0,                     /* tp_members */
    0,                                            /* tp_getset */
    0,                                            /* tp_base */
    0,                                            /* tp_dict */
    ( descrgetfunc )0,                            /* tp_descr_get */
    ( descrsetfunc )0,                            /* tp_descr_set */
    0,                                            /* tp_dictoffset */
    ( initproc )0,                                /* tp_init */
    ( allocfunc )PyType_GenericAlloc,             /* tp_alloc */
    ( newfunc )SortedMap_new,                     /* tp_new */
    ( freefunc )0,                                /* tp_free */
    ( inquiry )0,                                 /* tp_is_gc */
    0,                                            /* tp_bases */
    0,                                            /* tp_mro */
    0,                                            /* tp_cache */
    0,                                            /* tp_subclasses */
    0,                                            /* tp_weaklist */
    ( destructor )0                               /* tp_del */
};


static PyMethodDef
sortedmap_methods[] = {
    { 0 } // Sentinel
};


static struct PyModuleDef moduledef = {
        PyModuleDef_HEAD_INIT,
        "sortedmap",
        "sortedmap extension module",
        -1,
        sortedmap_methods,
        NULL,
        NULL,
        NULL,
        NULL
};


PyMODINIT_FUNC PyInit_sortedmap(void)
{
    PyObject *mod = PyModule_Create(&moduledef);
    if( !mod )
        return 0;
    if( PyType_Ready( &SortedMap_Type ) )
        return 0;
    Py_INCREF( ( PyObject* )( &SortedMap_Type ) );
    PyModule_AddObject( mod, "sortedmap", ( PyObject* )( &SortedMap_Type ) );

    return mod;
}
