/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include <cppy/cppy.h>
#include <utils/py23_str.h>
#include "atom.h"
#include "class_map.h"
#include "member.h"


namespace atom
{

namespace
{

PyObject* class_map_str;


PyObject* lookup_class_map( PyTypeObject* type )
{
    PyObject* py_map = PyDict_GetItem( type->tp_dict, class_map_str );
    if( py_map )
    {
        if( !ClassMap::TypeCheck( py_map ) )
        {
            return cppy::bad_internal_call( "class map has invalid type" );
        }
        return cppy::incref( py_map );
    }
    return cppy::bad_internal_call( "atom type has no class map" );
}


PyObject* Atom_new( PyTypeObject* type, PyObject* args, PyObject* kwargs )
{
    cppy::ptr map_ptr( lookup_class_map( type ) );
    if( !map_ptr )
    {
        return 0;
    }
    cppy::ptr self_ptr( PyType_GenericNew( type, args, kwargs ) );
    if( !self_ptr )
    {
        return 0;
    }
    Atom* atom = reinterpret_cast<Atom*>( self_ptr.get() );
    ClassMap* map = reinterpret_cast<ClassMap*>( map_ptr.get() );
    uint32_t count = map->getMemberCount();
    if( count > 0 )
    {
        size_t memsize = sizeof( PyObject* ) * count;
        void* slotmem = PyObject_Malloc( memsize );
        if( !slotmem )
        {
            return PyErr_NoMemory();
        }
        memset( slotmem, 0, memsize );
        atom->m_slots = reinterpret_cast<PyObject**>( slotmem );
    }
    atom->m_class_map = reinterpret_cast<ClassMap*>( map_ptr.release() );
    return self_ptr.release();
}


int Atom_init( PyObject* self, PyObject* args, PyObject* kwargs )
{
    if( PyTuple_GET_SIZE( args ) > 0 )
    {
        cppy::type_error( "__init__() takes no positional arguments" );
        return -1;
    }
    if( kwargs )
    {
        PyObject* key;
        PyObject* value;
        Py_ssize_t pos = 0;
        while( PyDict_Next( kwargs, &pos, &key, &value ) )
        {
            if( PyObject_SetAttr( self, key, value ) < 0 )
            {
                return -1;
            }
        }
    }
    return 0;
}


void Atom_clear( Atom* self )
{
    uint32_t count = self->m_class_map->getMemberCount();
    for( uint32_t i = 0; i < count; ++i )
    {
        Py_CLEAR( self->m_slots[ i ] );
    }
    Py_CLEAR( self->m_class_map );
}


int Atom_traverse( Atom* self, visitproc visit, void* arg )
{
    uint32_t count = self->m_class_map->getMemberCount();
    for( uint32_t i = 0; i < count; ++i )
    {
        Py_VISIT( self->m_slots[ i ] );
    }
    Py_VISIT( self->m_class_map );
    return 0;
}


void Atom_dealloc( Atom* self )
{
    PyObject_GC_UnTrack( self );
    Atom_clear( self );
    PyObject_Free( self->m_slots );
    self->ob_type->tp_free( reinterpret_cast<PyObject*>( self ) );
}


PyObject* Atom_getattro( PyObject* self, PyObject* name )
{
    uint32_t index;
    Member* member = 0;
    Atom* atom = reinterpret_cast<Atom*>( self );
    atom->m_class_map->getMember(
        // the interpreter guarantees this raw cast is safe
        reinterpret_cast<Py23StrObject*>( name ), &member, &index );
    if( member )
    {
        PyObject* value = atom->m_slots[ index ];
        if( value )
        {
            return cppy::incref( value );
        }
        value = member->getDefault( self, name );
        if( !value )
        {
            return 0;
        }
        atom->m_slots[ index ] = value;
        return cppy::incref( value );
    }
    return PyObject_GenericGetAttr( self, name );
}


int Atom_setattro( PyObject* self, PyObject* name, PyObject* val )
{
    if( !val )
    {
        // XXX handle deletes
        return 0;
    }
    uint32_t index;
    Member* member = 0;
    Atom* atom = reinterpret_cast<Atom*>( self );
    atom->m_class_map->getMember(
        // the interpreter guarantees this raw cast is safe
        reinterpret_cast<Py23StrObject*>( name ), &member, &index );
    if( member )
    {
        PyObject* old = atom->m_slots[ index ];
        if( old == val )
        {
            return 0;
        }
        val = member->validate( self, name, val );
        if( !val )
        {
            return -1;
        }
        atom->m_slots[ index ] = val;
        int result = member->postSetAttr( self, name, val );
        Py_XDECREF( old );
        return result;
    }
    return PyObject_GenericSetAttr( self, name, val );
}


PyObject* Atom_sizeof( Atom* self, PyObject* args )
{
    Py_ssize_t size = self->ob_type->tp_basicsize;
    size += sizeof( PyObject* ) * self->m_class_map->getMemberCount();
    return PyInt_FromSsize_t( size );
}


PyMethodDef Atom_methods[] = {
    { "__sizeof__",
      ( PyCFunction )Atom_sizeof,
      METH_NOARGS,
      "__sizeof__() -> size of object in memory, in bytes"},
    { 0 } // sentinel
};

} // namespace


PyTypeObject Atom::TypeObject = {
    PyObject_HEAD_INIT( &PyType_Type )
    0,                                   /* ob_size */
    "atom.catom.Atom",                   /* tp_name */
    sizeof( Atom ),                      /* tp_basicsize */
    0,                                   /* tp_itemsize */
    ( destructor )Atom_dealloc,          /* tp_dealloc */
    ( printfunc )0,                      /* tp_print */
    ( getattrfunc )0,                    /* tp_getattr */
    ( setattrfunc )0,                    /* tp_setattr */
    ( cmpfunc )0,                        /* tp_compare */
    ( reprfunc )0,                       /* tp_repr */
    ( PyNumberMethods* )0,               /* tp_as_number */
    ( PySequenceMethods* )0,             /* tp_as_sequence */
    ( PyMappingMethods* )0,              /* tp_as_mapping */
    ( hashfunc )0,                       /* tp_hash */
    ( ternaryfunc )0,                    /* tp_call */
    ( reprfunc )0,                       /* tp_str */
    ( getattrofunc )Atom_getattro,       /* tp_getattro */
    ( setattrofunc )Atom_setattro,       /* tp_setattro */
    ( PyBufferProcs* )0,                 /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT
    | Py_TPFLAGS_BASETYPE
    | Py_TPFLAGS_HAVE_GC
    | Py_TPFLAGS_HAVE_VERSION_TAG,       /* tp_flags */
    0,                                   /* Documentation string */
    ( traverseproc )Atom_traverse,       /* tp_traverse */
    ( inquiry )Atom_clear,               /* tp_clear */
    ( richcmpfunc )0,                    /* tp_richcompare */
    0,                                   /* tp_weaklistoffset */
    ( getiterfunc )0,                    /* tp_iter */
    ( iternextfunc )0,                   /* tp_iternext */
    ( struct PyMethodDef* )Atom_methods, /* tp_methods */
    ( struct PyMemberDef* )0,            /* tp_members */
    0,                                   /* tp_getset */
    0,                                   /* tp_base */
    0,                                   /* tp_dict */
    ( descrgetfunc )0,                   /* tp_descr_get */
    ( descrsetfunc )0,                   /* tp_descr_set */
    0,                                   /* tp_dictoffset */
    ( initproc )Atom_init,               /* tp_init */
    ( allocfunc )PyType_GenericAlloc,    /* tp_alloc */
    ( newfunc )Atom_new,                 /* tp_new */
    ( freefunc )PyObject_GC_Del,         /* tp_free */
    ( inquiry )0,                        /* tp_is_gc */
    0,                                   /* tp_bases */
    0,                                   /* tp_mro */
    0,                                   /* tp_cache */
    0,                                   /* tp_subclasses */
    0,                                   /* tp_weaklist */
    ( destructor )0                      /* tp_del */
};


bool Atom::Ready()
{
    class_map_str = Py23Str_FromString( "_[class map]" );
    if( !class_map_str )
    {
        return false;
    }
    return PyType_Ready( &TypeObject ) == 0;
}

} // namespace atom
