/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include <algorithm>
#include <cstring>
#include <cppy/cppy.h>
#include <utils/math.h>
#include "class_map.h"
#include "member.h"


namespace atom
{

struct ClassMapEntry
{
    Py23StrObject* name;
    Member* member;
    uint32_t index;
};


namespace
{

void insert_member( ClassMap* map, Py23StrObject* name, Member* member )
{
    uint32_t mask = map->m_allocated - 1;
    uint32_t hash = Py23Str_Hash( name );
    uint32_t bucket = hash & mask;
    ClassMapEntry* base = map->m_entries;
    while( true ) // table is never full, so always terminates in loop
    {
        // The table is pre-allocated with guaranteed sufficient space
        // and no two keys will be equal while populating the table.
        ClassMapEntry* entry = base + bucket;
        if( !entry->name )
        {
            entry->name = cppy::incref( name );
            entry->member = cppy::incref( member );
            entry->index = map->m_count++;
            return;
        }
        // CPython's collision resolution scheme
        bucket = ( ( bucket << 2 ) + bucket + hash + 1 ) & mask;
        hash >>= 5;
    };
}


PyObject* ClassMap_new( PyTypeObject* type, PyObject* args, PyObject* kwargs )
{
    static char* kwlist[] = { "members", 0 };
    static char* spec = "O:__new__";
    PyObject* members;
    if( !PyArg_ParseTupleAndKeywords( args, kwargs, spec, kwlist, &members ) )
    {
        return 0;
    }
    if( !PyDict_CheckExact( members ) )
    {
        return cppy::type_error( members, "dict" );
    }
    cppy::ptr self_ptr( PyType_GenericNew( type, 0, 0 ) );
    if( !self_ptr )
    {
        return 0;
    }
    static const uint32_t min_size = 4;
    uint32_t size = static_cast<uint32_t>( PyDict_Size( members ) );
    uint32_t allocated = utils::next_power_of_2( std::max( size, min_size ) );
    size_t mem_size = sizeof( ClassMapEntry ) * allocated;
    void* entries = PyObject_Malloc( mem_size );
    if( !entries )
    {
        return PyErr_NoMemory();
    }
    memset( entries, 0, mem_size );
    ClassMap* map = reinterpret_cast<ClassMap*>( self_ptr.get() );
    map->m_entries = reinterpret_cast<ClassMapEntry*>( entries );
    map->m_allocated = allocated;
    PyObject* key;
    PyObject* value;
    Py_ssize_t pos = 0;
    while( PyDict_Next( members, &pos, &key, &value ) )
    {
        if( !Py23Str_Check( key ) )
        {
            return cppy::type_error( key, "str" );
        }
        if( !Member::TypeCheck( value ) )
        {
            return cppy::type_error( value, "Member" );
        }
        insert_member( map,
            reinterpret_cast<Py23StrObject*>( key ),
            reinterpret_cast<Member*>( value ) );
    }
    return self_ptr.release();
}


void ClassMap_clear( ClassMap* self )
{
    uint32_t allocated = self->m_allocated;
    ClassMapEntry* base = self->m_entries;
    for( uint32_t i = 0; i < allocated; ++i )
    {
        ClassMapEntry* entry = base + i;
        if( entry->name )
        {
            Py_CLEAR( entry->name );
            Py_CLEAR( entry->member );
        }
    }
}


int ClassMap_traverse( ClassMap* self, visitproc visit, void* arg )
{
    uint32_t allocated = self->m_allocated;
    ClassMapEntry* base = self->m_entries;
    for( uint32_t i = 0; i < allocated; ++i )
    {
        ClassMapEntry* entry = base + i;
        if( entry->name )
        {
            Py_VISIT( entry->name );
            Py_VISIT( entry->member );
        }
    }
    return 0;
}


void ClassMap_dealloc( ClassMap* self )
{
    PyObject_GC_UnTrack( self );
    ClassMap_clear( self );
    PyObject_Free( self->m_entries );
    self->ob_type->tp_free( reinterpret_cast<PyObject*>( self ) );
}


PyObject* ClassMap_sizeof( ClassMap* self, PyObject* args )
{
    Py_ssize_t size = self->ob_type->tp_basicsize;
    size += sizeof( ClassMapEntry ) * self->m_allocated;
    return PyInt_FromSsize_t( size );
}


PyMethodDef ClassMap_methods[] = {
    { "__sizeof__",
      ( PyCFunction )ClassMap_sizeof,
      METH_NOARGS,
      "__sizeof__() -> size of object in memory, in bytes" },
    { 0 } // sentinel
};

} // namespace


PyTypeObject ClassMap::TypeObject = {
    PyObject_HEAD_INIT( &PyType_Type )       /* header */
    0,                                       /* ob_size */
    "atom.catom.ClassMap",                   /* tp_name */
    sizeof( ClassMap ),                      /* tp_basicsize */
    0,                                       /* tp_itemsize */
    ( destructor )ClassMap_dealloc,          /* tp_dealloc */
    ( printfunc )0,                          /* tp_print */
    ( getattrfunc )0,                        /* tp_getattr */
    ( setattrfunc )0,                        /* tp_setattr */
    ( cmpfunc )0,                            /* tp_compare */
    ( reprfunc )0,                           /* tp_repr */
    ( PyNumberMethods* )0,                   /* tp_as_number */
    ( PySequenceMethods* )0,                 /* tp_as_sequence */
    ( PyMappingMethods* )0,                  /* tp_as_mapping */
    ( hashfunc )0,                           /* tp_hash */
    ( ternaryfunc )0,                        /* tp_call */
    ( reprfunc )0,                           /* tp_str */
    ( getattrofunc )0,                       /* tp_getattro */
    ( setattrofunc )0,                       /* tp_setattro */
    ( PyBufferProcs* )0,                     /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC, /* tp_flags */
    0,                                       /* Documentation string */
    ( traverseproc )ClassMap_traverse,       /* tp_traverse */
    ( inquiry )ClassMap_clear,               /* tp_clear */
    ( richcmpfunc )0,                        /* tp_richcompare */
    0,                                       /* tp_weaklistoffset */
    ( getiterfunc )0,                        /* tp_iter */
    ( iternextfunc )0,                       /* tp_iternext */
    ( struct PyMethodDef* )ClassMap_methods, /* tp_methods */
    ( struct PyMemberDef* )0,                /* tp_members */
    0,                                       /* tp_getset */
    0,                                       /* tp_base */
    0,                                       /* tp_dict */
    ( descrgetfunc )0,                       /* tp_descr_get */
    ( descrsetfunc )0,                       /* tp_descr_set */
    0,                                       /* tp_dictoffset */
    ( initproc )0,                           /* tp_init */
    ( allocfunc )PyType_GenericAlloc,        /* tp_alloc */
    ( newfunc )ClassMap_new,                 /* tp_new */
    ( freefunc )PyObject_GC_Del,             /* tp_free */
    ( inquiry )0,                            /* tp_is_gc */
    0,                                       /* tp_bases */
    0,                                       /* tp_mro */
    0,                                       /* tp_cache */
    0,                                       /* tp_subclasses */
    0,                                       /* tp_weaklist */
    ( destructor )0                          /* tp_del */
};


bool ClassMap::Ready()
{
    return PyType_Ready( &TypeObject ) == 0;
}


void ClassMap::getMember( Py23StrObject* name, Member** member, uint32_t* index )
{
    uint32_t mask = m_allocated - 1;
    uint32_t hash = Py23Str_Hash( name );
    uint32_t bucket = hash & mask;
    ClassMapEntry* base = m_entries;
    while( true ) // table is never full, so always terminates in loop
    {
        ClassMapEntry* entry = base + bucket;
        if( !entry->name )
        {
            return;
        }
        if( Py23Str_Equal( name, entry->name ) )
        {
            *member = entry->member;
            *index = entry->index;
            return;
        }
        // CPython's collision resolution scheme
        bucket = ( ( bucket << 2 ) + bucket + hash + 1 ) & mask;
        hash >>= 5;
    };
}

} // namespace atom
