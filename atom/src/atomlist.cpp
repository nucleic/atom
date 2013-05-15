/*-----------------------------------------------------------------------------
| Copyright (c) 2013, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include "atomlist.h"


using namespace PythonHelpers;


namespace ListMethods
{

static PyCFunction append = 0;
static PyCFunction insert = 0;
static PyCFunction extend = 0;


static bool
init_methods()
{
    append = lookup_method( &PyList_Type, "append" );
    if( !append )
    {
        py_bad_internal_call( "failed to load list 'append' method" );
        return false;
    }
    insert = lookup_method( &PyList_Type, "insert" );
    if( !insert )
    {
        py_bad_internal_call( "failed to load list 'insert' method" );
        return false;
    }
    extend = lookup_method( &PyList_Type, "extend" );
    if( !extend )
    {
        py_bad_internal_call( "failed to load list 'extend' method" );
        return false;
    }
    return true;
}

}  // namespace ListMethods


static PyObject*
AtomList_new( PyTypeObject* type, PyObject* args, PyObject* kwargs )
{
    return AtomList_New( 0, 0, 0 );
}


static void
AtomList_dealloc( AtomList* self )
{
    delete self->pointer;
    self->pointer = 0;
    Py_CLEAR( self->validator );
    PyList_Type.tp_dealloc( pyobject_cast( self ) );
}


static PyObject*
validate_single( AtomList* self, PyObject* value )
{
    PyObjectPtr item( newref( value ) );
    if( self->validator && !self->pointer->is_null() )
    {
        item = self->validator->full_validate(
            self->pointer->data(), Py_None, item.get()
        );
        if( !item )
            return 0;
    }
    return item.release();
}


static PyObject*
validate_sequence( AtomList* self, PyObject* value )
{
    PyObjectPtr item( newref( value ) );
    if( self->validator && !self->pointer->is_null() )
    {
        // no validation needed for self[::-1] = self
        if( pyobject_cast( self ) != value )
        {
            PyListPtr templist( PySequence_List( value ) );
            if( !templist )
                return 0;
            Py_ssize_t size = templist.size();
            for( Py_ssize_t i = 0; i < size; ++i )
            {
                PyObject* val = self->validator->full_validate(
                    self->pointer->data(), Py_None, templist.borrow_item( i )
                );
                if( !val )
                    return 0;
                templist.set_item( i, val );
            }
            item = templist;
        }
    }
    return item.release();
}


static PyObject*
AtomList_append( AtomList* self, PyObject* value )
{
    PyObjectPtr item( validate_single( self, value ) );
    if( !item )
        return 0;
    return ListMethods::append( pyobject_cast( self ), item.get() );
}


static PyObject*
AtomList_insert( AtomList* self, PyObject* args )
{
    Py_ssize_t index;
    PyObject* value;
    if( !PyArg_ParseTuple( args, "nO:insert", &index, &value ) )
        return 0;
    value = validate_single( self, value );
    if( !value )
        return 0;
    PyObjectPtr item( PyTuple_New( 2 ) );
    if( !item )
        return 0;
    PyTuple_SET_ITEM( item.get(), 0, PyInt_FromSsize_t( index ) );
    PyTuple_SET_ITEM( item.get(), 1, value );
    return ListMethods::insert( pyobject_cast( self ), item.get() );
}


static PyObject*
AtomList_extend( AtomList* self, PyObject* value )
{
    PyObjectPtr item( validate_sequence( self, value ) );
    if( !item )
        return 0;
    return ListMethods::extend( pyobject_cast( self ), item.get() );
}


static PyObject*
AtomList_reduce_ex( AtomList* self, PyObject* proto )
{
    // An atomlist is pickled as a normal list. When the Atom class is
    // reconstituted, assigning the list to the attribute will create
    // a new atomlist with the proper owner. There is no need to try
    // to persist the validator and pointer information.
    PyObjectPtr data( PySequence_List( pyobject_cast( self ) ) );
    if( !data )
        return 0;
    PyTuplePtr res( PyTuple_New( 2 ) );
    if( !res )
        return 0;
    PyTuplePtr args( PyTuple_New( 1 ) );
    if( !args )
        return 0;
    args.set_item( 0, data );
    res.set_item( 0, newref( pyobject_cast( &PyList_Type ) ) );
    res.set_item( 1, args );
    return res.release();
}


static int
AtomList_ass_item( AtomList* self, Py_ssize_t index, PyObject* value )
{
    if( !value )
        return PyList_Type.tp_as_sequence->sq_ass_item(
            pyobject_cast( self ), index, value );
    PyObjectPtr item( validate_single( self, value ) );
    if( !item )
        return -1;
    return PyList_Type.tp_as_sequence->sq_ass_item(
        pyobject_cast( self ), index, item.get() );
}


static int
AtomList_ass_slice( AtomList* self, Py_ssize_t low, Py_ssize_t high, PyObject* value )
{
    if( !value )
        return PyList_Type.tp_as_sequence->sq_ass_slice(
            pyobject_cast( self ), low, high, value );
    PyObjectPtr item( validate_sequence( self, value ) );
    if( !item )
        return -1;
    return PyList_Type.tp_as_sequence->sq_ass_slice(
        pyobject_cast( self ), low, high, item.get() );
}


static PyObject*
AtomList_inplace_concat( AtomList* self, PyObject* value )
{
    PyObjectPtr item( validate_sequence( self, value ) );
    if( !item )
        return 0;
    return PyList_Type.tp_as_sequence->sq_inplace_concat(
        pyobject_cast( self ), item.get() );
}


static int
AtomList_ass_subscript( AtomList* self, PyObject* key, PyObject* value )
{
    if( !value )
        return PyList_Type.tp_as_mapping->mp_ass_subscript(
            pyobject_cast( self ), key, value );
    PyObjectPtr item;
    if( PyIndex_Check( key ) )
        item = validate_single( self, value );
    else if( PySlice_Check( key ) )
        item = validate_sequence( self, value );
    else
        item = newref( value );
    return PyList_Type.tp_as_mapping->mp_ass_subscript(
        pyobject_cast( self ), key, value );
}


PyDoc_STRVAR(append_doc,
"L.append(object) -- append object to end");
PyDoc_STRVAR(insert_doc,
"L.insert(index, object) -- insert object before index");
PyDoc_STRVAR(extend_doc,
"L.extend(iterable) -- extend list by appending elements from the iterable");


static PyMethodDef
AtomList_methods[] = {
    { "append", ( PyCFunction )AtomList_append, METH_O, append_doc },
    { "insert", ( PyCFunction )AtomList_insert, METH_VARARGS, insert_doc },
    { "extend", ( PyCFunction )AtomList_extend, METH_O, extend_doc },
    { "__reduce_ex__", ( PyCFunction )AtomList_reduce_ex, METH_O, "" },
    { 0 }  /* sentinel */
};


static PySequenceMethods
AtomList_as_sequence = {
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


static PyMappingMethods
AtomList_as_mapping = {
    (lenfunc)0,                             /* mp_length */
    (binaryfunc)0,                          /* mp_subscript */
    (objobjargproc)AtomList_ass_subscript   /* mp_ass_subscript */
};


PyTypeObject AtomList_Type = {
    PyObject_HEAD_INIT( &PyType_Type )
    0,                                      /* ob_size */
    "catom.atomlist",                       /* tp_name */
    sizeof( AtomList ),                     /* tp_basicsize */
    0,                                      /* tp_itemsize */
    (destructor)AtomList_dealloc,           /* tp_dealloc */
    (printfunc)0,                           /* tp_print */
    (getattrfunc)0,                         /* tp_getattr */
    (setattrfunc)0,                         /* tp_setattr */
    (cmpfunc)0,                             /* tp_compare */
    (reprfunc)0,                            /* tp_repr */
    (PyNumberMethods*)0,                    /* tp_as_number */
    (PySequenceMethods*)&AtomList_as_sequence, /* tp_as_sequence */
    (PyMappingMethods*)&AtomList_as_mapping,   /* tp_as_mapping */
    (hashfunc)0,                            /* tp_hash */
    (ternaryfunc)0,                         /* tp_call */
    (reprfunc)0,                            /* tp_str */
    (getattrofunc)0,                        /* tp_getattro */
    (setattrofunc)0,                        /* tp_setattro */
    (PyBufferProcs*)0,                      /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE, /* tp_flags */
    0,                                      /* Documentation string */
    (traverseproc)0,                        /* tp_traverse */
    (inquiry)0,                             /* tp_clear */
    (richcmpfunc)0,                         /* tp_richcompare */
    0,                                      /* tp_weaklistoffset */
    (getiterfunc)0,                         /* tp_iter */
    (iternextfunc)0,                        /* tp_iternext */
    (struct PyMethodDef*)AtomList_methods,  /* tp_methods */
    (struct PyMemberDef*)0,                 /* tp_members */
    0,                                      /* tp_getset */
    &PyList_Type,                           /* tp_base */
    0,                                      /* tp_dict */
    (descrgetfunc)0,                        /* tp_descr_get */
    (descrsetfunc)0,                        /* tp_descr_set */
    0,                                      /* tp_dictoffset */
    (initproc)0,                            /* tp_init */
    (allocfunc)0,                           /* tp_alloc */
    (newfunc)AtomList_new,                  /* tp_new */
    (freefunc)0,                            /* tp_free */
    (inquiry)0,                             /* tp_is_gc */
    0,                                      /* tp_bases */
    0,                                      /* tp_mro */
    0,                                      /* tp_cache */
    0,                                      /* tp_subclasses */
    0,                                      /* tp_weaklist */
    (destructor)0                           /* tp_del */
};


PyObject*
AtomList_New( Py_ssize_t size, CAtom* atom, Member* validator )
{
    // The list's internal pointer array can't be malloced directly,
    // or the internal list_resize method will blow up when two heaps
    // are in use. This can happen when using this extension compiled
    // with MinGW on a Python compiled with MSVCC, for example. So, a
    // slave list is allocated and it's pointer array is stolen. This
    // ensures the pointer array is malloced, resized, and freed by
    // the same CRT.
    PyObjectPtr slave( PyList_New( size ) );
    if( !slave )
        return 0;

    PyObjectPtr ptr( PyType_GenericNew( &AtomList_Type, 0, 0 ) );
    if( !ptr )
        return 0;

    PyListObject* op = reinterpret_cast<PyListObject*>( ptr.get() );
    PyListObject* slave_op = reinterpret_cast<PyListObject*>( slave.get() );
    op->ob_item = slave_op->ob_item;
    op->allocated = size;
    Py_SIZE( op ) = size;

    slave_op->ob_item = 0;
    slave_op->allocated = 0;
    Py_SIZE( slave_op ) = 0;

    Py_XINCREF( pyobject_cast( validator ) );
    atomlist_cast( ptr.get() )->validator = validator;
    atomlist_cast( ptr.get() )->pointer = new CAtomPointer( atom );
    return ptr.release();
}


int
import_atomlist()
{
    if( PyType_Ready( &AtomList_Type ) < 0 )
        return -1;
    if( !ListMethods::init_methods() )
        return -1;
    return 0;
}
