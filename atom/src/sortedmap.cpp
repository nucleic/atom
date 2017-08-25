/*-----------------------------------------------------------------------------
| Copyright (c) 2013, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include <algorithm>
#include <vector>
#include <iostream>
#include <sstream>
#include "pythonhelpers.h"
#include "py23compat.h"

using namespace PythonHelpers;

class MapItem
{

public:

    MapItem() {}

    MapItem( PyObject* key, PyObject* value ) :
        m_key( newref( key ) ), m_value( newref( value ) ) { }

    MapItem( PyObjectPtr& key, PyObjectPtr& value ) :
        m_key( key ), m_value( value ) { }

    MapItem( PyObjectPtr& key, PyObject* value ) :
        m_key( key ), m_value( newref( value ) ) { }

    MapItem( PyObject* key, PyObjectPtr& value ) :
        m_key( newref( key ) ), m_value( value ) { }

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
        m_value = newref( value );
    }

    struct CmpLess
    {
        // All three operators are needed in order to keep the
        // MSVC debug version of std::lower_bound happy.
        bool operator()( MapItem& first, MapItem& second )
        {
            if( first.m_key == second.m_key )
                return false;
            return first.m_key.richcompare( second.m_key, Py_LT );
        }

        bool operator()( MapItem& first, PyObject* second )
        {
            if( first.m_key == second )
                return false;
            return first.m_key.richcompare( second, Py_LT );
        }

        bool operator()( PyObject* first, MapItem& second )
        {
            if( first == second.m_key )
                return false;
            PyObjectPtr temp( newref( first ) );
            return temp.richcompare( second.m_key, Py_LT );
        }
    };

    struct CmpEq
    {
        bool operator()( MapItem& first, MapItem& second )
        {
            if( first.m_key == second.m_key )
                return true;
            return first.m_key.richcompare( second.m_key, Py_EQ );
        }

        bool operator()( MapItem& first, PyObject* second )
        {
            if( first.m_key == second )
                return true;
            return first.m_key.richcompare( second, Py_EQ );
        }

        bool operator()( PyObject* first, MapItem& second )
        {
            if( first == second.m_key )
                return true;
            PyObjectPtr temp( newref( first ) );
            return temp.richcompare( second.m_key, Py_EQ );
        }
    };

private:

    PyObjectPtr m_key;
    PyObjectPtr m_value;
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
                return newref( default_value );
            return lookup_fail( key );
        }
        if( MapItem::CmpEq()( *it, key ) )
            return newref( it->value() );
        if( default_value )
            return newref( default_value );
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
                return newref( default_value );
            return lookup_fail( key );
        }
        if( MapItem::CmpEq()( *it, key ) )
        {
            PyObject* res = newref( it->value() );
            m_items->erase( it );
            return res;
        }
        if( default_value )
            return newref( default_value );
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
            PyList_SET_ITEM( pylist, listidx, newref( it->key() ) );
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
            PyList_SET_ITEM( pylist, listidx, newref( it->value() ) );
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
            PyTuple_SET_ITEM( pytuple, 0, newref( it->key() ) );
            PyTuple_SET_ITEM( pytuple, 1, newref( it->value() ) );
            PyList_SET_ITEM( pylist, listidx, pytuple );
            ++listidx;
        }
        return pylist;
    }

    static PyObject* lookup_fail( PyObject* key )
    {
        PyObjectPtr pystr( PyObject_Str( key ) );
        if( !pystr )
            return 0;
        PyObjectPtr pytuple( PyTuple_Pack( 1, key ) );
        if (!pytuple)
            return 0;
        PyErr_SetObject(PyExc_KeyError, pytuple.get());
        return 0;
    }
};


static PyObject*
SortedMap_new( PyTypeObject* type, PyObject* args, PyObject* kwargs )
{
    PyObject* self = PyType_GenericNew( type, args, kwargs );
    if( !self )
        return 0;
    SortedMap* cself = reinterpret_cast<SortedMap*>( self );
    cself->m_items = new SortedMap::Items();
    return self;
}

#if PY_MAJOR_VERSION >= 3
    static int
    SortedMap_clear( SortedMap* self )
    {
        SortedMap::Items empty;
        self->m_items->swap( empty );
        return 0;
    }
#else
    static void
    SortedMap_clear( SortedMap* self )
    {
        // Clearing the vector may cause arbitrary side effects on item
        // decref, including calls into methods which mutate the vector.
        // To avoid segfaults, first make the vector empty, then let the
        // destructors run for the old items.
        SortedMap::Items empty;
        self->m_items->swap( empty );
    }
#endif


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
    0,                          /* sq_length */
    0,                          /* sq_concat */
    0,                          /* sq_repeat */
    0,                          /* sq_item */
    0,                          /* sq_slice */
    0,                          /* sq_ass_item */
    0,                          /* sq_ass_slice */
    ( objobjproc )SortedMap_contains, /* sq_contains */
    0,                          /* sq_inplace_concat */
    0,                          /* sq_inplace_repeat */
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
    return py_type_fail( ostr.str().c_str() );
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
    return py_type_fail( ostr.str().c_str() );
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
    ostr << "sortedmap({";
    SortedMap::Items::iterator it;
    SortedMap::Items::iterator end_it = self->m_items->end();
    for( it = self->m_items->begin(); it != end_it; ++it )
    {
        PyObjectPtr keystr( PyObject_Str( it->key() ) );
        if( !keystr )
            return 0;
        PyObjectPtr valstr( PyObject_Str( it->value() ) );
        if( !valstr )
            return 0;
        ostr << Py23Str_AS_STRING( keystr.get() ) << ": ";
        ostr << Py23Str_AS_STRING( valstr.get() ) << ", ";
    }
    if( self->m_items->size() > 0 )
        ostr.seekp( -2, std::ios_base::cur );
    ostr << "})";
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
    return Py23Int_FromSsize_t( size );
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
    "sortedmap.sortedmap",                  /* tp_name */
    sizeof( SortedMap ),                    /* tp_basicsize */
    0,                                      /* tp_itemsize */
    (destructor)SortedMap_dealloc,          /* tp_dealloc */
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
    (reprfunc)SortedMap_repr,               /* tp_repr */
    (PyNumberMethods*)0,                    /* tp_as_number */
    (PySequenceMethods*)&SortedMap_as_sequence, /* tp_as_sequence */
    (PyMappingMethods*)&SortedMap_as_mapping,   /* tp_as_mapping */
    (hashfunc)0,                            /* tp_hash */
    (ternaryfunc)0,                         /* tp_call */
    (reprfunc)0,                            /* tp_str */
    (getattrofunc)0,                        /* tp_getattro */
    (setattrofunc)0,                        /* tp_setattro */
    (PyBufferProcs*)0,                      /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC, /* tp_flags */
    0,                                      /* Documentation string */
    (traverseproc)SortedMap_traverse,       /* tp_traverse */
    (inquiry)SortedMap_clear,               /* tp_clear */
    (richcmpfunc)0,                         /* tp_richcompare */
    0,                                      /* tp_weaklistoffset */
    (getiterfunc)0,                         /* tp_iter */
    (iternextfunc)0,                        /* tp_iternext */
    (struct PyMethodDef*)SortedMap_methods, /* tp_methods */
    (struct PyMemberDef*)0,                 /* tp_members */
    0,                                      /* tp_getset */
    0,                                      /* tp_base */
    0,                                      /* tp_dict */
    (descrgetfunc)0,                        /* tp_descr_get */
    (descrsetfunc)0,                        /* tp_descr_set */
    0,                                      /* tp_dictoffset */
    (initproc)0,                            /* tp_init */
    (allocfunc)PyType_GenericAlloc,         /* tp_alloc */
    (newfunc)SortedMap_new,                 /* tp_new */
    (freefunc)0,                            /* tp_free */
    (inquiry)0,                             /* tp_is_gc */
    0,                                      /* tp_bases */
    0,                                      /* tp_mro */
    0,                                      /* tp_cache */
    0,                                      /* tp_subclasses */
    0,                                      /* tp_weaklist */
    (destructor)0                           /* tp_del */
};


static PyMethodDef
sortedmap_methods[] = {
    { 0 } // Sentinel
};

#if PY_MAJOR_VERSION >= 3

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

#define INITERROR return NULL
#define MOD_INIT(name) PyMODINIT_FUNC PyInit_##name(void)

#else

#define INITERROR return
#define MOD_INIT(name) PyMODINIT_FUNC init##name(void)

#endif

MOD_INIT( sortedmap )
{
#if PY_MAJOR_VERSION >= 3
    PyObject *mod = PyModule_Create(&moduledef);
#else
    PyObject* mod = Py_InitModule( "sortedmap", sortedmap_methods );
#endif
    if( !mod )
        INITERROR;
    if( PyType_Ready( &SortedMap_Type ) )
        INITERROR;
    Py_INCREF( ( PyObject* )( &SortedMap_Type ) );
    PyModule_AddObject( mod, "sortedmap", ( PyObject* )( &SortedMap_Type ) );

#if PY_MAJOR_VERSION >= 3
    return mod;
#endif
}
