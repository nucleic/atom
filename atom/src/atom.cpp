/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include <atom.h>

#include <class_map.h>
#include <member.h>
#include <utils/py23compat.h>

#include <cppy/cppy.h>


namespace atom
{

namespace
{

PyObject* class_map_str;


PyObject* lookup_class_map( PyTypeObject* type )
{
    PyObject* class_map = PyDict_GetItem( type->tp_dict, class_map_str );
    if( !class_map )
    {
        return cppy::system_error( "atom type has no class map" );
    }
    if( !ClassMap::TypeCheck( class_map ) )
    {
        return cppy::system_error( "atom class map has invalid type" );
    }
    return cppy::incref( class_map );
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

    Atom* self = reinterpret_cast<Atom*>( self_ptr.get() );
    ClassMap* map = reinterpret_cast<ClassMap*>( map_ptr.get() );
    uint32_t member_count = map->memberCount();
    if( member_count > 0 )
    {
        size_t mem_size = sizeof( PyObject* ) * member_count;
        void* slot_mem = PyObject_Malloc( mem_size );
        if( !slot_mem )
        {
            return PyErr_NoMemory();
        }
        memset( slot_mem, 0, mem_size );
        self->m_slots = reinterpret_cast<PyObject**>( slot_mem );
    }
    self->m_class_map = reinterpret_cast<ClassMap*>( map_ptr.release() );

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


int Atom_clear( Atom* self )
{
    uint32_t count = self->m_class_map->memberCount();
    for( uint32_t i = 0; i < count; ++i )
    {
        Py_CLEAR( self->m_slots[ i ] );
    }
    Py_CLEAR( self->m_class_map );
    return 0;
}


int Atom_traverse( Atom* self, visitproc visit, void* arg )
{
    uint32_t count = self->m_class_map->memberCount();
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
    cppy::ptr name_ptr( PyName_As23Str( name ) );
    if( !name_ptr )
    {
        return 0;
    }
    uint32_t index;
    Member* member = 0;
    Atom* atom = reinterpret_cast<Atom*>( self );
    atom->m_class_map->member(
        reinterpret_cast<Py23StrObject*>( name_ptr.get() ), &member, &index );
    if( member )
    {
        PyObject* value = atom->m_slots[ index ];
        if( value )
        {
            return cppy::incref( value );
        }
        value = member->defaultValue( self, name_ptr.get() );
        if( !value )
        {
            return 0;
        }
        atom->m_slots[ index ] = value;
        return cppy::incref( value );
    }
    return PyObject_GenericGetAttr( self, name_ptr.get() );
}


int Atom_setattro( PyObject* self, PyObject* name, PyObject* value )
{
    cppy::ptr name_ptr( PyName_As23Str( name ) );
    if( !name_ptr )
    {
        return -1;
    }
    uint32_t index;
    Member* member = 0;
    Atom* atom = reinterpret_cast<Atom*>( self );
    atom->m_class_map->member(
        reinterpret_cast<Py23StrObject*>( name_ptr.get() ), &member, &index );
    if( member )
    {
        PyObject* old = atom->m_slots[ index ];
        if( value == old )
        {
            return 0;
        }
        if( !value )
        {
            // XXX handle deletes
            return 0;
        }
        value = member->validateValue( self, name_ptr.get(), value );
        if( !value )
        {
            return -1;
        }
        atom->m_slots[ index ] = value; // 'old' is now an owned ref
        int result = member->postSetAttrValue( self, name_ptr.get(), value );
        cppy::xdecref( old );
        return result;
    }
    return PyObject_GenericSetAttr( self, name_ptr.get(), value );
}


PyObject* Atom_sizeof( Atom* self, PyObject* args )
{
    Py_ssize_t size = self->ob_type->tp_basicsize;
    size += sizeof( PyObject* ) * self->m_class_map->memberCount();
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
    "atom.catom.CAtom",                  /* tp_name */
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
