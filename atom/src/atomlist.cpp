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
static PyCFunction pop = 0;
static PyCFunction remove = 0;
static PyCFunction reverse = 0;
static PyCFunctionWithKeywords sort = 0;


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
    pop = lookup_method( &PyList_Type, "pop" );
    if( !pop )
    {
        py_bad_internal_call( "failed to load list 'pop' method" );
        return false;
    }
    remove = lookup_method( &PyList_Type, "remove" );
    if( !remove )
    {
        py_bad_internal_call( "failed to load list 'remove' method" );
        return false;
    }
    reverse = lookup_method( &PyList_Type, "reverse" );
    if( !reverse )
    {
        py_bad_internal_call( "failed to load list 'reverse' method" );
        return false;
    }
    typedef PyCFunctionWithKeywords func_t;
    sort = reinterpret_cast<func_t>( lookup_method( &PyList_Type, "sort" ) );
    if( !sort )
    {
        py_bad_internal_call( "failed to load list 'sort' method" );
        return false;
    }
    return true;
}

}  // namespace ListMethods


static PyObject*
ListSubtype_New( PyTypeObject* subtype, Py_ssize_t size )
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

    PyObjectPtr ptr( PyType_GenericNew( subtype, 0, 0 ) );
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

    return ptr.release();
}


PyObject*
AtomList_New( Py_ssize_t size, CAtom* atom, Member* validator )
{
    PyObjectPtr ptr( ListSubtype_New( &AtomList_Type, size ) );
    if( !ptr )
        return 0;
    Py_XINCREF( pyobject_cast( validator ) );
    atomlist_cast( ptr.get() )->validator = validator;
    atomlist_cast( ptr.get() )->pointer = new CAtomPointer( atom );
    return ptr.release();
}


PyObject*
AtomCList_New( Py_ssize_t size, CAtom* atom, Member* validator, Member* member )
{
    PyObjectPtr ptr( ListSubtype_New( &AtomCList_Type, size ) );
    if( !ptr )
        return 0;
    Py_XINCREF( pyobject_cast( validator ) );
    Py_XINCREF( pyobject_cast( member ) );
    atomlist_cast( ptr.get() )->validator = validator;
    atomlist_cast( ptr.get() )->pointer = new CAtomPointer( atom );
    atomclist_cast( ptr.get() )->member = member;
    return ptr.release();
}


/*-----------------------------------------------------------------------------
| AtomList Type
|----------------------------------------------------------------------------*/
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


PyDoc_STRVAR(a_append_doc,
"L.append(object) -- append object to end");
PyDoc_STRVAR(a_insert_doc,
"L.insert(index, object) -- insert object before index");
PyDoc_STRVAR(a_extend_doc,
"L.extend(iterable) -- extend list by appending elements from the iterable");


static PyMethodDef
AtomList_methods[] = {
    { "append", ( PyCFunction )AtomList_append, METH_O, a_append_doc },
    { "insert", ( PyCFunction )AtomList_insert, METH_VARARGS, a_insert_doc },
    { "extend", ( PyCFunction )AtomList_extend, METH_O, a_extend_doc },
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


/*-----------------------------------------------------------------------------
| AtomCList Type
|----------------------------------------------------------------------------*/
namespace PySStr
{


class PyStringMaker
{

public:

    PyStringMaker( const char* string ) : m_pystring( 0 )
    {
        m_pystring = PyString_FromString( string );
    }

    PyObject* operator()()
    {
        return m_pystring.get();
    }

private:

    PyStringMaker();
    PyObjectPtr m_pystring;
};


#define _STATIC_STRING( name )                \
    static PyObject*                          \
    name()                                    \
    {                                         \
        static PyStringMaker string( #name ); \
        return string();                      \
    }

_STATIC_STRING( type )
_STATIC_STRING( name )
_STATIC_STRING( object )
_STATIC_STRING( value )
_STATIC_STRING( operation )
_STATIC_STRING( item )
_STATIC_STRING( items )
_STATIC_STRING( index )
_STATIC_STRING( cmp )
_STATIC_STRING( key )
_STATIC_STRING( reverse )
_STATIC_STRING( container )
_STATIC_STRING( __delitem__ )
_STATIC_STRING( __iadd__ )
_STATIC_STRING( __imul__ )
_STATIC_STRING( __setitem__ )
_STATIC_STRING( append )
_STATIC_STRING( extend )
_STATIC_STRING( insert )
_STATIC_STRING( pop )
_STATIC_STRING( remove )
_STATIC_STRING( sort )

}  // namespace PySStr


static PyObject*
AtomCList_new( PyTypeObject* type, PyObject* args, PyObject* kwargs )
{
    return AtomCList_New( 0, 0, 0, 0 );
}


static void
AtomCList_dealloc( AtomCList* self )
{
    Py_CLEAR( self->member );
    AtomList_dealloc( atomlist_cast( self ) );
}


namespace
{

inline bool
observer_check( AtomCList* self, bool& obs_m, bool& obs_a )
{
    obs_m = self->member->has_observers();
    obs_a = atomlist_cast( self )->pointer->data()->has_observers( self->member->name );
    return obs_m || obs_a;
}


inline PyObject*
prepare_change( AtomCList* self )
{
    PyDictPtr c( PyDict_New() );
    if( !c )
        return 0;
    if( !c.set_item( PySStr::type(), PySStr::container() ) )
        return 0;
    if( !c.set_item( PySStr::name(), self->member->name ) )
        return 0;
    CAtomPointer* pointer = atomlist_cast( self )->pointer;
    if( !c.set_item( PySStr::object(), pyobject_cast( pointer->data() ) ) )
        return 0;
    if( !c.set_item( PySStr::value(), pyobject_cast( self ) ) )
        return 0;
    return c.release();
}


inline bool
post_change( AtomCList* self, PyObjectPtr& change, bool obs_m, bool obs_a )
{
    PyTuplePtr args( PyTuple_New( 1 ) );
    if( !args )
        return false;
    args.set_item( 0, change );
    CAtomPointer* pointer = atomlist_cast( self )->pointer;
    if( obs_m )
        if( !self->member->notify( pointer->data(), args.get(), 0 ) )
            return false;
    if( obs_a )
        if( !pointer->data()->notify( self->member->name, args.get(), 0 ) )
            return false;
    return true;
}


}  // namespace


static PyObject*
AtomCList_append( AtomCList* self, PyObject* value )
{
    PyObjectPtr res( AtomList_append( atomlist_cast( self ), value ) );
    if( !res )
        return 0;
    if( self->member && !atomlist_cast( self )->pointer->is_null() )
    {
        bool obs_m, obs_a;
        if( observer_check( self, obs_m, obs_a ) )
        {
            PyDictPtr c( prepare_change( self ) );
            if( !c )
                return 0;
            if( !c.set_item( PySStr::operation(), PySStr::append() ) )
                return 0;
            Py_ssize_t index = PyList_GET_SIZE( self ) - 1;
            if( !c.set_item( PySStr::item(), PyList_GET_ITEM( self, index ) ) )
                return 0;
            if( !post_change( self, c, obs_m, obs_a ) )
                return 0;
        }
    }
    return res.release();
}


static PyObject*
AtomCList_insert( AtomCList* self, PyObject* args )
{
    Py_ssize_t size = PyList_GET_SIZE( self );
    PyObjectPtr res( AtomList_insert( atomlist_cast( self ), args ) );
    if( !res )
        return 0;
    if( self->member && !atomlist_cast( self )->pointer->is_null() )
    {
        bool obs_m, obs_a;
        if( observer_check( self, obs_m, obs_a ) )
        {
            PyDictPtr c( prepare_change( self ) );
            if( !c )
                return 0;
            if( !c.set_item( PySStr::operation(), PySStr::insert() ) )
                return 0;
            // if the superclass call succeeds, then this is safe.
            Py_ssize_t where = PyInt_AsSsize_t( PyTuple_GET_ITEM( args, 0 ) );
            if( where < 0 )
            {
                where += size;
                if( where < 0 )
                    where = 0;
            }
            if( where > size )
                where = size;
            PyObjectPtr index( PyInt_FromSsize_t( where ) );
            if( !c.set_item( PySStr::index(), index.get() ) )
                return 0;
            if( !c.set_item( PySStr::item(), PyList_GET_ITEM( self, where ) ) )
                return 0;
            if( !post_change( self, c, obs_m, obs_a ) )
                return 0;
        }
    }
    return res.release();
}


static PyObject*
AtomCList_extend( AtomCList* self, PyObject* value )
{
    Py_ssize_t size = PyList_GET_SIZE( self );
    PyObjectPtr res( AtomList_extend( atomlist_cast( self ), value ) );
    if( !res )
        return 0;
    if( self->member && !atomlist_cast( self )->pointer->is_null() )
    {
        bool obs_m, obs_a;
        if( observer_check( self, obs_m, obs_a ) )
        {
            PyDictPtr c( prepare_change( self ) );
            if( !c )
                return 0;
            if( !c.set_item( PySStr::operation(), PySStr::extend() ) )
                return 0;
            Py_ssize_t newsize = PyList_GET_SIZE( self );
            PyObjectPtr items( PyList_GetSlice( pyobject_cast( self ), size, newsize ) );
            if( !items )
                return 0;
            if( !c.set_item( PySStr::items(), items.get() ) )
                return 0;
            if( !post_change( self, c, obs_m, obs_a ) )
                return 0;
        }
    }
    return res.release();
}


static PyObject*
AtomCList_pop( AtomCList* self, PyObject* args )
{
    PyObjectPtr res( ListMethods::pop( pyobject_cast( self ), args ) );
    if( !res )
        return 0;
    if( self->member && !atomlist_cast( self )->pointer->is_null() )
    {
        bool obs_m, obs_a;
        if( observer_check( self, obs_m, obs_a ) )
        {
            PyDictPtr c( prepare_change( self ) );
            if( !c )
                return 0;
            if( !c.set_item( PySStr::operation(), PySStr::pop() ) )
                return 0;
            // if the superclass call succeeds, then this is safe.
            Py_ssize_t i = -1;
            if( PyTuple_GET_SIZE( args ) == 1 )
                i = PyInt_AsSsize_t( PyTuple_GET_ITEM( args, 0 ) );
            if( i < 0 )
                i += PyList_GET_SIZE( self ) + 1;
            PyObjectPtr index( PyInt_FromSsize_t( i ) );
            if( !c.set_item( PySStr::index(), index.get() ) )
                return 0;
            if( !c.set_item( PySStr::item(), res.get() ) )
                return 0;
            if( !post_change( self, c, obs_m, obs_a ) )
                return 0;
        }
    }
    return res.release();
}


static PyObject*
AtomCList_remove( AtomCList* self, PyObject* value )
{
    PyObjectPtr res( ListMethods::remove( pyobject_cast( self ), value ) );
    if( !res )
        return 0;
    if( self->member && !atomlist_cast( self )->pointer->is_null() )
    {
        bool obs_m, obs_a;
        if( observer_check( self, obs_m, obs_a ) )
        {
            PyDictPtr c( prepare_change( self ) );
            if( !c )
                return 0;
            if( !c.set_item( PySStr::operation(), PySStr::remove() ) )
                return 0;
            if( !c.set_item( PySStr::item(), value ) )
                return 0;
            if( !post_change( self, c, obs_m, obs_a ) )
                return 0;
        }
    }
    return res.release();
}


static PyObject*
AtomCList_reverse( AtomCList* self )
{
    PyObjectPtr res( ListMethods::reverse( pyobject_cast( self ), 0 ) );
    if( !res )
        return 0;
    if( self->member && !atomlist_cast( self )->pointer->is_null() )
    {
        bool obs_m, obs_a;
        if( observer_check( self, obs_m, obs_a ) )
        {
            PyDictPtr c( prepare_change( self ) );
            if( !c )
                return 0;
            if( !c.set_item( PySStr::operation(), PySStr::reverse() ) )
                return 0;
            if( !post_change( self, c, obs_m, obs_a ) )
                return 0;
        }
    }
    return res.release();
}


static PyObject*
AtomCList_sort( AtomCList* self, PyObject* args, PyObject* kwargs )
{
    static char *kwlist[] = { "cmp", "key", "reverse", 0 };
    PyObjectPtr res( ListMethods::sort( pyobject_cast( self ), args, kwargs ) );
    if( !res )
        return 0;
    if( self->member && !atomlist_cast( self )->pointer->is_null() )
    {
        bool obs_m, obs_a;
        if( observer_check( self, obs_m, obs_a ) )
        {
            PyDictPtr c( prepare_change( self ) );
            if( !c )
                return 0;
            if( !c.set_item( PySStr::operation(), PySStr::sort() ) )
                return 0;
            PyObject* cmp = Py_None;
            PyObject* key = Py_None;
            int reverse = 0;
            // if the superclass call succeeds, then this is safe.
            PyArg_ParseTupleAndKeywords(
                args, kwargs, "|OOi", kwlist, &cmp, &key, &reverse
            );
            if( !c.set_item( PySStr::cmp(), cmp ) )
                return 0;
            if( !c.set_item( PySStr::key(), key ) )
                return 0;
            if( !c.set_item( PySStr::reverse(), reverse ? Py_True : Py_False ) )
                return 0;
            if( !post_change( self, c, obs_m, obs_a ) )
                return 0;
        }
    }
    return res.release();
}


// static int
// AtomCList_ass_item( AtomCList* self, Py_ssize_t index, PyObject* value )
// {
//     if( !value )
//         return PyList_Type.tp_as_sequence->sq_ass_item(
//             pyobject_cast( self ), index, value );
//     PyObjectPtr item( validate_single( self, value ) );
//     if( !item )
//         return -1;
//     return PyList_Type.tp_as_sequence->sq_ass_item(
//         pyobject_cast( self ), index, item.get() );
// }


// static int
// AtomCList_ass_slice( AtomCList* self, Py_ssize_t low, Py_ssize_t high, PyObject* value )
// {
//     if( !value )
//         return PyList_Type.tp_as_sequence->sq_ass_slice(
//             pyobject_cast( self ), low, high, value );
//     PyObjectPtr item( validate_sequence( self, value ) );
//     if( !item )
//         return -1;
//     return PyList_Type.tp_as_sequence->sq_ass_slice(
//         pyobject_cast( self ), low, high, item.get() );
// }


// static PyObject*
// AtomCList_inplace_concat( AtomCList* self, PyObject* value )
// {
//     PyObjectPtr item( validate_sequence( self, value ) );
//     if( !item )
//         return 0;
//     return PyList_Type.tp_as_sequence->sq_inplace_concat(
//         pyobject_cast( self ), item.get() );
// }


// static int
// AtomCList_ass_subscript( AtomCList* self, PyObject* key, PyObject* value )
// {
//     if( !value )
//         return PyList_Type.tp_as_mapping->mp_ass_subscript(
//             pyobject_cast( self ), key, value );
//     PyObjectPtr item;
//     if( PyIndex_Check( key ) )
//         item = validate_single( self, value );
//     else if( PySlice_Check( key ) )
//         item = validate_sequence( self, value );
//     else
//         item = newref( value );
//     return PyList_Type.tp_as_mapping->mp_ass_subscript(
//         pyobject_cast( self ), key, value );
// }


PyDoc_STRVAR(c_append_doc,
"L.append(object) -- append object to end");
PyDoc_STRVAR(c_insert_doc,
"L.insert(index, object) -- insert object before index");
PyDoc_STRVAR(c_extend_doc,
"L.extend(iterable) -- extend list by appending elements from the iterable");
PyDoc_STRVAR(c_pop_doc,
"L.pop([index]) -> item -- remove and return item at index (default last).\n"
"Raises IndexError if list is empty or index is out of range.");
PyDoc_STRVAR(c_remove_doc,
"L.remove(value) -- remove first occurrence of value.\n"
"Raises ValueError if the value is not present.");
PyDoc_STRVAR(c_reverse_doc,
"L.reverse() -- reverse *IN PLACE*");
PyDoc_STRVAR(c_sort_doc,
"L.sort(cmp=None, key=None, reverse=False) -- stable sort *IN PLACE*;\n\
cmp(x, y) -> -1, 0, 1");


static PyMethodDef
AtomCList_methods[] = {
    { "append", ( PyCFunction )AtomCList_append, METH_O, c_append_doc },
    { "insert", ( PyCFunction )AtomCList_insert, METH_VARARGS, c_insert_doc },
    { "extend", ( PyCFunction )AtomCList_extend, METH_O, c_extend_doc },
    { "pop", ( PyCFunction )AtomCList_pop, METH_VARARGS, c_pop_doc },
    { "remove", ( PyCFunction )AtomCList_remove, METH_O, c_remove_doc },
    { "reverse", ( PyCFunction )AtomCList_reverse, METH_NOARGS, c_reverse_doc },
    { "sort", ( PyCFunction )AtomCList_sort, METH_VARARGS | METH_KEYWORDS, c_sort_doc },
    { 0 }  /* sentinel */
};


// static PySequenceMethods
// AtomCList_as_sequence = {
//     (lenfunc)0,                                 /* sq_length */
//     (binaryfunc)0,                              /* sq_concat */
//     (ssizeargfunc)0,                            /* sq_repeat */
//     (ssizeargfunc)0,                            /* sq_item */
//     (ssizessizeargfunc)0,                       /* sq_slice */
//     (ssizeobjargproc)AtomCList_ass_item,         /* sq_ass_item */
//     (ssizessizeobjargproc)AtomCList_ass_slice,   /* sq_ass_slice */
//     (objobjproc)0,                              /* sq_contains */
//     (binaryfunc)AtomCList_inplace_concat,        /* sq_inplace_concat */
//     (ssizeargfunc)0,                            /* sq_inplace_repeat */
// };


// static PyMappingMethods
// AtomCList_as_mapping = {
//     (lenfunc)0,                             /* mp_length */
//     (binaryfunc)0,                          /* mp_subscript */
//     (objobjargproc)AtomCList_ass_subscript   /* mp_ass_subscript */
// };


PyTypeObject AtomCList_Type = {
    PyObject_HEAD_INIT( &PyType_Type )
    0,                                      /* ob_size */
    "catom.atomclist",                      /* tp_name */
    sizeof( AtomCList ),                    /* tp_basicsize */
    0,                                      /* tp_itemsize */
    (destructor)AtomCList_dealloc,          /* tp_dealloc */
    (printfunc)0,                           /* tp_print */
    (getattrfunc)0,                         /* tp_getattr */
    (setattrfunc)0,                         /* tp_setattr */
    (cmpfunc)0,                             /* tp_compare */
    (reprfunc)0,                            /* tp_repr */
    (PyNumberMethods*)0,                    /* tp_as_number */
    (PySequenceMethods*)0,//&AtomCList_as_sequence, /* tp_as_sequence */
    (PyMappingMethods*)0,//&AtomCList_as_mapping,   /* tp_as_mapping */
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
    (struct PyMethodDef*)AtomCList_methods, /* tp_methods */
    (struct PyMemberDef*)0,                 /* tp_members */
    0,                                      /* tp_getset */
    &AtomList_Type,                         /* tp_base */
    0,                                      /* tp_dict */
    (descrgetfunc)0,                        /* tp_descr_get */
    (descrsetfunc)0,                        /* tp_descr_set */
    0,                                      /* tp_dictoffset */
    (initproc)0,                            /* tp_init */
    (allocfunc)0,                           /* tp_alloc */
    (newfunc)AtomCList_new,                 /* tp_new */
    (freefunc)0,                            /* tp_free */
    (inquiry)0,                             /* tp_is_gc */
    0,                                      /* tp_bases */
    0,                                      /* tp_mro */
    0,                                      /* tp_cache */
    0,                                      /* tp_subclasses */
    0,                                      /* tp_weaklist */
    (destructor)0                           /* tp_del */
};


int
import_atomlist()
{
    if( PyType_Ready( &AtomList_Type ) < 0 )
        return -1;
    if( PyType_Ready( &AtomCList_Type ) < 0 )
        return -1;
    if( !ListMethods::init_methods() )
        return -1;
    return 0;
}
