/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include <atom_list.h>

#include <member.h>

#include <cppy/cppy.h>


namespace atom
{

namespace
{

PyObject* get_formatter()
{
    static PyObject* the_function = 0;
    if( !the_function )
    {
        cppy::ptr mod( PyImport_ImportModule( "atom.formatting" ) );
        if( !mod )
        {
            return 0;
        }
        the_function = PyObject_GetAttrString( mod.get(), "element_message" );
        if( !the_function )
        {
            return 0;
        }
    }
    return the_function;
}


PyObject* validate_single(
    Member* member, PyObject* atom, PyObject* name, PyObject* item )
{
    cppy::ptr value( member->validateValue( atom, name, item ) );
    if( value )
    {
        return value.release();
    }
    if( PyErr_ExceptionMatches( ValidationError ) )
    {
        PyErr_Clear();
        PyObject* formatter = get_formatter();
        if( !formatter )
        {
            return 0;
        }
        cppy::ptr args( PyTuple_Pack( 4, member, atom, name, item ) );
        if( !args )
        {
            return 0;
        }
        cppy::ptr message( PyObject_Call( formatter, args.get(), 0 ) );
        if( !message )
        {
            return 0;
        }
        PyErr_SetObject( ValidationError, message.get() );
    }
    return 0;
}


PyObject* validate_single( AtomList* list, PyObject* item )
{
    Member* member = list->m_member;
    PyObject* name = list->m_name;
    cppy::ptr atom( PyWeakref_GET_OBJECT( list->m_atomref ), true );
    return validate_single( member, atom.get(), name, item );
}


PyObject* validate_iterable( AtomList* list, PyObject* iter )
{
    Member* member = list->m_member;
    PyObject* name = list->m_name;
    cppy::ptr atom( PyWeakref_GET_OBJECT( list->m_atomref ), true );
    cppy::ptr result( PyList_New( 0 ) );
    if( !result )
    {
        return 0;
    }
    cppy::ptr item;
    while( item = PyIter_Next( iter ) )
    {
        item = validate_single( member, atom.get(), name, item.get() );
        if( !item )
        {
            return 0;
        }
        if( PyList_Append( result.get(), item.get() ) != 0 )
        {
            return 0;
        }
    }
    if( PyErr_Occurred() )
    {
        return 0;
    }
    return result.release();
}


int AtomList_clear( AtomList* self )
{
    Py_CLEAR( self->m_member );
    Py_CLEAR( self->m_atomref );
    Py_CLEAR( self->m_name );
    return PyList_Type.tp_clear( reinterpret_cast<PyObject*>( self ) );
}


int AtomList_traverse( AtomList* self, visitproc visit, void* arg )
{
    Py_VISIT( self->m_member );
    Py_VISIT( self->m_atomref );
    Py_VISIT( self->m_name );
    return PyList_Type.tp_traverse(
        reinterpret_cast<PyObject*>( self ), visit, arg );
}


void AtomList_dealloc( AtomList* self )
{
    PyObject_GC_UnTrack( self );
    AtomList_clear( self );
    PyList_Type.tp_dealloc( reinterpret_cast<PyObject*>( self ) );
}


PyObject* AtomList_append( AtomList* self, PyObject* item )
{
    cppy::ptr value( validate_single( self, item ) );
    if( !value )
    {
        return 0;
    }
    PyObject* list = reinterpret_cast<PyObject*>( self );
    if( PyList_Append( list, value.get() ) != 0 )
    {
        return 0;
    }
    return cppy::incref( Py_None );
}


PyObject* AtomList_insert( AtomList* self, PyObject* args )
{
    Py_ssize_t index;
    PyObject* item;
    if( !PyArg_ParseTuple( args, "nO:insert", &index, &item ) )
    {
        return 0;
    }
    cppy::ptr value( validate_single( self, item ) );
    if( !value )
    {
        return 0;
    }
    PyObject* list = reinterpret_cast<PyObject*>( self );
    if( PyList_Insert( list, index, value.get() ) != 0 )
    {
        return 0;
    }
    return cppy::incref( Py_None );
}


PyObject* AtomList_extend( AtomList* self, PyObject* item )
{
    cppy::ptr iter( PyObject_GetIter( item ) );
    if( !iter )
    {
        return 0;
    }
    cppy::ptr values( validate_iterable( self, iter.get() ) );
    if( !values )
    {
        return 0;
    }
    PyListObject* list = reinterpret_cast<PyListObject*>( self );
    return _PyList_Extend( list, values.get() );
}


PyObject* AtomList_reduce_ex( PyObject* self, PyObject* proto )
{
    // An AtomList is pickled as a normal list. When the Atom class is
    // reconstituted, assigning the list to the attribute will create
    // a new AtomList with the proper owner. There is no need to try
    // to persist any of the extra information.
    cppy::ptr data( PyList_GetSlice( self, 0, PyList_GET_SIZE( self ) ) );
    if( !data )
    {
        return 0;
    }
    cppy::ptr args( PyTuple_Pack( 1, data.get() ) );
    if( !args )
    {
        return 0;
    }
    PyObject* type = reinterpret_cast<PyObject*>( &PyList_Type );
    return PyTuple_Pack( 2, type, args.get() );
}


int AtomList_ass_item( AtomList* self, Py_ssize_t index, PyObject* item )
{
    PyObject* d = reinterpret_cast<PyObject*>( self );
    if( !item )
    {
        return PyList_Type.tp_as_sequence->sq_ass_item( d, index, item );
    }
    cppy::ptr value( validate_single( self, item ) );
    if( !value )
    {
        return -1;
    }
    return PyList_Type.tp_as_sequence->sq_ass_item( d, index, value.get() );
}


int AtomList_ass_slice(
    AtomList* self, Py_ssize_t low, Py_ssize_t high, PyObject* item )
{
    PyObject* d = reinterpret_cast<PyObject*>( self );
    if( !item )
    {
        return PyList_Type.tp_as_sequence->sq_ass_slice( d, low, high, item );
    }
    cppy::ptr iter( PyObject_GetIter( item ) );
    if( !iter )
    {
        return -1;
    }
    cppy::ptr val( validate_iterable( self, iter.get() ) );
    if( !val )
    {
        return -1;
    }
    return PyList_Type.tp_as_sequence->sq_ass_slice( d, low, high, val.get() );
}


PyObject* AtomList_inplace_concat( AtomList* self, PyObject* value )
{
    cppy::ptr res( AtomList_extend( self, value ) );
    if( !res )
    {
        return 0;
    }
    return cppy::incref( reinterpret_cast<PyObject*>( self ) );
}


int AtomList_ass_subscript( AtomList* self, PyObject* key, PyObject* item )
{
    PyObject* d = reinterpret_cast<PyObject*>( self );
    if( !item )
    {
        return PyList_Type.tp_as_mapping->mp_ass_subscript( d, key, item );
    }
    cppy::ptr value( validate_single( self, item ) );
    if( !value )
    {
        return -1;
    }
    return PyList_Type.tp_as_mapping->mp_ass_subscript( d, key, value.get() );
}


PyDoc_STRVAR( append_doc,
"L.append(object) -- append object to end" );

PyDoc_STRVAR( insert_doc,
"L.insert(index, object) -- insert object before index" );

PyDoc_STRVAR( extend_doc,
"L.extend(iterable) -- extend list by appending elements from the iterable" );


static PyMethodDef
AtomList_methods[] = {
    { "append", ( PyCFunction )AtomList_append, METH_O, append_doc },
    { "insert", ( PyCFunction )AtomList_insert, METH_VARARGS, insert_doc },
    { "extend", ( PyCFunction )AtomList_extend, METH_O, extend_doc },
    { "__reduce_ex__", ( PyCFunction )AtomList_reduce_ex, METH_O, "" },
    { 0 }  /* sentinel */
};


PySequenceMethods AtomList_as_sequence = {
    (lenfunc)0,                                 /* sq_length */
    (binaryfunc)0,                              /* sq_concat */
    (ssizeargfunc)0,                            /* sq_repeat */
    (ssizeargfunc)0,                            /* sq_item */
    (ssizessizeargfunc)0,                       /* sq_slice */
    (ssizeobjargproc)AtomList_ass_item,         /* sq_ass_item */
    (ssizessizeobjargproc)AtomList_ass_slice,   /* sq_ass_slice */
    (objobjproc)0,                              /* sq_contains */
    (binaryfunc)AtomList_inplace_concat,        /* sq_inplace_concat */
    (ssizeargfunc)0,                            /* sq_inplace_repeat */
};


PyMappingMethods AtomList_as_mapping = {
    (lenfunc)0,                             /* mp_length */
    (binaryfunc)0,                          /* mp_subscript */
    (objobjargproc)AtomList_ass_subscript   /* mp_ass_subscript */
};

} // namespace


PyTypeObject AtomList::TypeObject = {
    PyObject_HEAD_INIT( &PyType_Type )
    0,                                           /* ob_size */
    "atom.catom.AtomList",                       /* tp_name */
    sizeof( AtomList ),                          /* tp_basicsize */
    0,                                           /* tp_itemsize */
    ( destructor )AtomList_dealloc,              /* tp_dealloc */
    ( printfunc )0,                              /* tp_print */
    ( getattrfunc )0,                            /* tp_getattr */
    ( setattrfunc )0,                            /* tp_setattr */
    ( cmpfunc )0,                                /* tp_compare */
    ( reprfunc )0,                               /* tp_repr */
    ( PyNumberMethods* )0,                       /* tp_as_number */
    ( PySequenceMethods* )&AtomList_as_sequence, /* tp_as_sequence */
    ( PyMappingMethods* )&AtomList_as_mapping,   /* tp_as_mapping */
    ( hashfunc )0,                               /* tp_hash */
    ( ternaryfunc )0,                            /* tp_call */
    ( reprfunc )0,                               /* tp_str */
    ( getattrofunc )0,                           /* tp_getattro */
    ( setattrofunc )0,                           /* tp_setattro */
    ( PyBufferProcs* )0,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT
    | Py_TPFLAGS_HAVE_GC
    | Py_TPFLAGS_HAVE_VERSION_TAG,               /* tp_flags */
    0,                                           /* Documentation string */
    ( traverseproc )AtomList_traverse,           /* tp_traverse */
    ( inquiry )AtomList_clear,                   /* tp_clear */
    ( richcmpfunc )0,                            /* tp_richcompare */
    0,                                           /* tp_weaklistoffset */
    ( getiterfunc )0,                            /* tp_iter */
    ( iternextfunc )0,                           /* tp_iternext */
    ( struct PyMethodDef* )AtomList_methods,     /* tp_methods */
    ( struct PyMemberDef* )0,                    /* tp_members */
    0,                                           /* tp_getset */
    &PyList_Type,                                /* tp_base */
    0,                                           /* tp_dict */
    ( descrgetfunc )0,                           /* tp_descr_get */
    ( descrsetfunc )0,                           /* tp_descr_set */
    0,                                           /* tp_dictoffset */
    ( initproc )0,                               /* tp_init */
    ( allocfunc )PyType_GenericAlloc,            /* tp_alloc */
    ( newfunc )0,                                /* tp_new */
    ( freefunc )PyObject_GC_Del,                 /* tp_free */
    ( inquiry )0,                                /* tp_is_gc */
    0,                                           /* tp_bases */
    0,                                           /* tp_mro */
    0,                                           /* tp_cache */
    0,                                           /* tp_subclasses */
    0,                                           /* tp_weaklist */
    ( destructor )0                              /* tp_del */
};


bool AtomList::Ready()
{
    return PyType_Ready( &TypeObject ) == 0;
}


PyObject* AtomList::Create(
    Member* member, PyObject* atom, PyObject* name, PyObject* items )
{
    cppy::ptr result( PyType_GenericNew( &TypeObject, 0, 0 ) );
    if( !result )
    {
        return 0;
    }
    PyListObject* pylist = reinterpret_cast<PyListObject*>( result.get() );
    cppy::ptr ignored( _PyList_Extend( pylist, items ) );
    if( !ignored )
    {
        return 0;
    }
    AtomList* list = reinterpret_cast<AtomList*>( result.get() );
    list->m_atomref = PyWeakref_NewRef( atom, 0 );
    if( !list->m_atomref )
    {
        return 0;
    }
    list->m_member = cppy::incref( member );
    list->m_name = cppy::incref( name );
    return result.release();
}

} // namespace atom
