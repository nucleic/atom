/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2019, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include <cppy/cppy.h>
#include "atomlist.h"
#include "packagenaming.h"

#ifdef __clang__
#pragma clang diagnostic ignored "-Wdeprecated-writable-strings"
#endif

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif

typedef PyCFunction pycfunc;
typedef PyCFunctionWithKeywords pycfunc_kw;
#if PY_VERSION_HEX >= 0x03070000
typedef _PyCFunctionFast pycfunc_f;
typedef _PyCFunctionFastWithKeywords pycfunc_fkw;
#endif

namespace ListMethods
{
pycfunc extend = 0;
#if PY_VERSION_HEX >= 0x03070000
static pycfunc_f pop = 0;
#else
static pycfunc pop = 0;
#endif
static pycfunc remove = 0;
#if PY_VERSION_HEX >= 0x03070000
static pycfunc_fkw sort = 0;
#else
static pycfunc_kw sort = 0;
#endif

inline PyCFunction
lookup_method( PyTypeObject* type, const char* name )
{
    PyMethodDef* method = type->tp_methods;
    for( ; method->ml_name != 0; ++method )
    {
        if( strcmp( method->ml_name, name ) == 0 )
            return method->ml_meth;
    }
    return 0;
}

static bool
init_methods()
{

    extend = lookup_method( &PyList_Type, "extend" );
    if( !extend )
    {
// LCOV_EXCL_START
        cppy::system_error( "failed to load list 'extend' method" );
        return false;
// LCOV_EXCL_STOP
    }
#if PY_VERSION_HEX >= 0x03070000
    pop = reinterpret_cast<pycfunc_f>( lookup_method( &PyList_Type, "pop" ) );
#else
    pop = lookup_method( &PyList_Type, "pop" );
#endif
    if( !pop )
    {
// LCOV_EXCL_START
        cppy::system_error( "failed to load list 'pop' method" );
        return false;
// LCOV_EXCL_STOP
    }
    remove = lookup_method( &PyList_Type, "remove" );
    if( !remove )
    {
// LCOV_EXCL_START
        cppy::system_error( "failed to load list 'remove' method" );
        return false;
// LCOV_EXCL_STOP
    }
#if PY_VERSION_HEX >= 0x03070000
    sort = reinterpret_cast<pycfunc_fkw>( lookup_method( &PyList_Type, "sort" ) );
#else
    sort = reinterpret_cast<pycfunc_kw>( lookup_method( &PyList_Type, "sort" ) );
#endif

    if( !sort )
    {
// LCOV_EXCL_START
        cppy::system_error( "failed to load list 'sort' method" );
        return false;
// LCOV_EXCL_STOP
    }
    return true;
}

}  // namespace ListMethods


static PyObject*
ListSubtype_New( PyTypeObject* subtype, Py_ssize_t size )
{
    if( size < 0 )
        return cppy::system_error( "negative list size" );
    if( static_cast<size_t>( size ) > PY_SSIZE_T_MAX / sizeof( PyObject* ) )
        return PyErr_NoMemory();  // LCOV_EXCL_LINE
    cppy::ptr ptr( PyType_GenericNew( subtype, 0, 0 ) );
    if( !ptr )
        return 0;
    PyListObject* op = reinterpret_cast<PyListObject*>( ptr.get() );
    if( size > 0 )
    {
        size_t nbytes = size * sizeof( PyObject* );
        op->ob_item = reinterpret_cast<PyObject**>( PyMem_Malloc( nbytes ) );
        if( !op->ob_item )
            return PyErr_NoMemory();  // LCOV_EXCL_LINE
        memset( op->ob_item, 0, nbytes );
    }
    Py_SIZE( op ) = size;
    op->allocated = size;
    return ptr.release();
}


PyObject*
AtomList_New( Py_ssize_t size, CAtom* atom, Member* validator )
{
    cppy::ptr ptr( ListSubtype_New( &AtomList_Type, size ) );
    if( !ptr )
        return 0;
    cppy::xincref( pyobject_cast( validator ) );
    atomlist_cast( ptr.get() )->validator = validator;
    atomlist_cast( ptr.get() )->pointer = new CAtomPointer( atom );
    return ptr.release();
}


PyObject*
AtomCList_New( Py_ssize_t size, CAtom* atom, Member* validator, Member* member )
{
    cppy::ptr ptr( ListSubtype_New( &AtomCList_Type, size ) );
    if( !ptr )
        return 0;
    cppy::xincref( pyobject_cast( validator ) );
    cppy::xincref( pyobject_cast( member ) );
    atomlist_cast( ptr.get() )->validator = validator;
    atomlist_cast( ptr.get() )->pointer = new CAtomPointer( atom );
    atomclist_cast( ptr.get() )->member = member;
    return ptr.release();
}


/*-----------------------------------------------------------------------------
| AtomList Type
|----------------------------------------------------------------------------*/
namespace
{

class AtomListHandler
{

public:

    AtomListHandler( AtomList* list ) :
        m_list( cppy::incref( pyobject_cast( list ) ) ) {}

    PyObject* append( PyObject* value )
    {
        cppy::ptr item( validate_single( value ) );
        if( !item )
            return 0;
        if( PyList_Append( m_list.get(), item.get() ) != 0 )
        {
            return 0;
        }
        return cppy::incref( Py_None );
    }

    PyObject* insert( PyObject* args )
    {
        Py_ssize_t index;
        PyObject* value;
        if( !PyArg_ParseTuple( args, "nO:insert", &index, &value ) )
            return 0;
        cppy::ptr valptr( validate_single( value ) );
        if( !valptr )
            return 0;
        if( PyList_Insert( m_list.get(), index, valptr.get() ) != 0)
        {
            return 0;
        }
        return cppy::incref( Py_None );

    }

    PyObject* extend( PyObject* value )
    {
        cppy::ptr item( validate_sequence( value ) );
        if( !item )
            return 0;
        return ListMethods::extend( m_list.get(), item.get() );
    }

    PyObject* iadd( PyObject* value )
    {
        cppy::ptr item( validate_sequence( value ) );
        if( !item )
            return 0;
        return PyList_Type.tp_as_sequence->sq_inplace_concat(
            m_list.get(), item.get() );
    }

    int setitem( Py_ssize_t index, PyObject* value )
    {
        if( !value )
            return PyList_Type.tp_as_sequence->sq_ass_item(
                m_list.get(), index, value );
        cppy::ptr item( validate_single( value ) );
        if( !item )
            return -1;
        return PyList_Type.tp_as_sequence->sq_ass_item(
            m_list.get(), index, item.get() );
    }

    int setitem( PyObject* key, PyObject* value )
    {
        if( !value )
            return PyList_Type.tp_as_mapping->mp_ass_subscript(
                m_list.get(), key, value );
        cppy::ptr item;
        if( PyIndex_Check( key ) )
            item = validate_single( value );
        else if( PySlice_Check( key ) )
            item = validate_sequence( value );
        else
            item = cppy::incref( value );
        if( !item )
            return -1;
        return PyList_Type.tp_as_mapping->mp_ass_subscript(
            m_list.get(), key, item.get() );
    }

protected:

    AtomList* alist()
    {
        return atomlist_cast( m_list.get() );
    }

    Member* validator()
    {
        return alist()->validator;
    }

    CAtom* atom()
    {
        return alist()->pointer->data();
    }

    PyObject* validate_single( PyObject* value )
    {
        cppy::ptr item( cppy::incref( value ) );
        if( validator() && atom() )
        {
            item = validator()->full_validate( atom(), Py_None, item.get() );
            if( !item )
                return 0;
        }
        m_validated = item;
        return item.release();
    }

    PyObject* validate_sequence( PyObject* value )
    {
        cppy::ptr item( cppy::incref( value ) );
        if( validator() && atom() )
        {
            // no validation needed for self[::-1] = self
            if( m_list.get() != value )
            {
                cppy::ptr templist( PySequence_List( value ) );
                if( !templist )
                    return 0;
                CAtom* atm = atom();
                Member* vd = validator();
                Py_ssize_t size = PyList_GET_SIZE( templist.get() );
                for( Py_ssize_t i = 0; i < size; ++i )
                {
                    // Borrow a reference to an item in the list
                    PyObject* b = PyList_GET_ITEM( templist.get(), i );
                    PyObject* val = vd->full_validate( atm, Py_None, b );
                    if( !val )
                        return 0;
                    PyList_SET_ITEM( templist.get(), i, cppy::incref( val ) );
                }
                item = templist;
            }
        }
        m_validated = item;
        return item.release();
    }

    cppy::ptr m_list;
    cppy::ptr m_validated;

private:

    AtomListHandler();
};

}  // namespace


static PyObject*
AtomList_new( PyTypeObject* type, PyObject* args, PyObject* kwargs )
{
    cppy::ptr ptr( PyList_Type.tp_new( type, args, kwargs ) );
    if( !ptr )
        return 0;
    atomlist_cast( ptr.get() )->pointer = new CAtomPointer();
    return ptr.release();
}

int AtomList_clear( AtomList* self )
{
	Py_CLEAR( self->validator );
	return PyList_Type.tp_clear( pyobject_cast( self ) );
}


int AtomList_traverse( AtomList* self, visitproc visit, void* arg )
{
	Py_VISIT( self->validator );
	return PyList_Type.tp_traverse( pyobject_cast( self ), visit, arg );
}

static void
AtomList_dealloc( AtomList* self )
{
    PyObject_GC_UnTrack( self );
    delete self->pointer;
    self->pointer = 0;
    Py_CLEAR( self->validator );
    PyList_Type.tp_dealloc( pyobject_cast( self ) );
}


static PyObject*
AtomList_append( AtomList* self, PyObject* value )
{
    return AtomListHandler( self ).append( value );
}


static PyObject*
AtomList_insert( AtomList* self, PyObject* args )
{
    return AtomListHandler( self ).insert( args );
}


static PyObject*
AtomList_extend( AtomList* self, PyObject* value )
{
    return AtomListHandler( self ).extend( value );
}


static PyObject*
AtomList_reduce_ex( AtomList* self, PyObject* proto )
{
    // An atomlist is pickled as a normal list. When the Atom class is
    // reconstituted, assigning the list to the attribute will create
    // a new atomlist with the proper owner. There is no need to try
    // to persist the validator and pointer information.
    cppy::ptr data( PySequence_List( pyobject_cast( self ) ) );
    if( !data )
        return 0;
    cppy::ptr res( PyTuple_New( 2 ) );
    if( !res )
        return 0;
    cppy::ptr args( PyTuple_New( 1 ) );
    if( !args )
        return 0;
    PyTuple_SET_ITEM( args.get(), 0, data.release() );
    PyTuple_SET_ITEM( res.get(), 0, cppy::incref( pyobject_cast( &PyList_Type ) ) );
    PyTuple_SET_ITEM( res.get(), 1, args.release() );
    return res.release();
}


static int
AtomList_ass_item( AtomList* self, Py_ssize_t index, PyObject* value )
{
    return AtomListHandler( self ).setitem( index, value );
}


static PyObject*
AtomList_inplace_concat( AtomList* self, PyObject* value )
{
    return AtomListHandler( self ).iadd( value );
}


static int
AtomList_ass_subscript( AtomList* self, PyObject* key, PyObject* value )
{
    return AtomListHandler( self ).setitem( key, value );
}


PyDoc_STRVAR( a_append_doc,
"L.append(object) -- append object to end" );

PyDoc_STRVAR( a_insert_doc,
"L.insert(index, object) -- insert object before index" );

PyDoc_STRVAR( a_extend_doc,
"L.extend(iterable) -- extend list by appending elements from the iterable" );


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
    ( lenfunc )0,                                 /* sq_length */
    ( binaryfunc )0,                              /* sq_concat */
    ( ssizeargfunc )0,                            /* sq_repeat */
    ( ssizeargfunc )0,                            /* sq_item */
    ( void * )0,                                  /* sq_slice */
    ( ssizeobjargproc )AtomList_ass_item,         /* sq_ass_item */
    ( void * )0,                                  /* sq_ass_slice */
    ( objobjproc )0,                              /* sq_contains */
    ( binaryfunc )AtomList_inplace_concat,        /* sq_inplace_concat */
    ( ssizeargfunc )0,                            /* sq_inplace_repeat */
};


static PyMappingMethods
AtomList_as_mapping = {
    ( lenfunc )0,                             /* mp_length */
    ( binaryfunc )0,                          /* mp_subscript */
    ( objobjargproc )AtomList_ass_subscript   /* mp_ass_subscript */
};


PyTypeObject AtomList_Type = {
    PyVarObject_HEAD_INIT( &PyType_Type, 0 )
    PACKAGE_TYPENAME( "atomlist" ),              /* tp_name */
    sizeof( AtomList ),                          /* tp_basicsize */
    0,                                           /* tp_itemsize */
    ( destructor )AtomList_dealloc,              /* tp_dealloc */
    ( printfunc )0,                              /* tp_print */
    ( getattrfunc )0,                            /* tp_getattr */
    ( setattrfunc )0,                            /* tp_setattr */
	( PyAsyncMethods* )0,                        /* tp_as_async */
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
    |Py_TPFLAGS_BASETYPE
    |Py_TPFLAGS_HAVE_GC,                         /* tp_flags */
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
    ( allocfunc )0,                              /* tp_alloc */
    ( newfunc )AtomList_new,                     /* tp_new */
    ( freefunc )0,                               /* tp_free */
    ( inquiry )0,                                /* tp_is_gc */
    0,                                           /* tp_bases */
    0,                                           /* tp_mro */
    0,                                           /* tp_cache */
    0,                                           /* tp_subclasses */
    0,                                           /* tp_weaklist */
    ( destructor )0                              /* tp_del */
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
        m_pystring = PyUnicode_FromString( string );
    }

    PyObject* operator()()
    {
        return m_pystring.get();
    }

private:

    PyStringMaker();
    cppy::ptr m_pystring;
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
_STATIC_STRING( olditem )
_STATIC_STRING( newitem )
_STATIC_STRING( count )

}  // namespace PySStr


namespace
{

class AtomCListHandler : public AtomListHandler
{

    static void clip_index( Py_ssize_t& index, Py_ssize_t size )
    {
        if( index < 0 )
        {
            index += size;
            if( index < 0 )
                index = 0;
        }
        if( index > size )
            index = size;
    }

// XXX should I add clear ?
public:

    AtomCListHandler( AtomCList* list ) :
        AtomListHandler( atomlist_cast( list ) ),
        m_obsm( false ), m_obsa( false ) {}

    PyObject* append( PyObject* value )
    {
        cppy::ptr res( AtomListHandler::append( value ) );
        if( !res )
            return 0;
        if( observer_check() )
        {
            cppy::ptr c( prepare_change() );
            if( !c )
                return 0;  // LCOV_EXCL_LINE
            if( PyDict_SetItem( c.get(), PySStr::operation(), PySStr::append() ) != 0 )
                return 0;
            if( PyDict_SetItem( c.get(), PySStr::item(), m_validated.get() ) != 0 )
                return 0;
            if( !post_change( c ) )
                return 0;
        }
        return res.release();
    }

    PyObject* insert( PyObject* args )
    {
        Py_ssize_t size = PyList_GET_SIZE( m_list.get() );
        cppy::ptr res( AtomListHandler::insert( args ) );
        if( !res )
            return 0;
        if( observer_check() )
        {
            cppy::ptr c( prepare_change() );
            if( !c )
                return 0;  // LCOV_EXCL_LINE
            if( PyDict_SetItem( c.get(), PySStr::operation(), PySStr::insert() ) != 0 )
                return 0;
            // if the superclass call succeeds, then this is safe.
            Py_ssize_t where = PyLong_AsSsize_t( PyTuple_GET_ITEM( args, 0 ) );
            clip_index( where, size );
            cppy::ptr index( PyLong_FromSsize_t( where ) );
            if( PyDict_SetItem( c.get(), PySStr::index(), index.get() ) != 0 )
                return 0;
            if( PyDict_SetItem( c.get(), PySStr::item(), m_validated.get() ) != 0)
                return 0;
            if( !post_change( c ) )
                return 0;
        }
        return res.release();
    }

    PyObject* extend( PyObject* value )
    {
        cppy::ptr res( AtomListHandler::extend( value ) );
        if( !res )
            return 0;
        if( observer_check() )
        {
            cppy::ptr c( prepare_change() );
            if( !c )
                return 0;  // LCOV_EXCL_LINE
            if( PyDict_SetItem( c.get(), PySStr::operation(), PySStr::extend() ) != 0 )
                return 0;
            if( PyDict_SetItem( c.get(), PySStr::items(), m_validated.get() ) != 0 )
                return 0;
            if( !post_change( c ) )
                return 0;
        }
        return res.release();
    }

    PyObject* pop( PyObject* args )
    {
        Py_ssize_t size = PyList_GET_SIZE( m_list.get() );
#if PY_VERSION_HEX >= 0x03070000
        int nargs = (int)PyTuple_GET_SIZE( args);
        PyObject **stack = &PyTuple_GET_ITEM(args, 0);
        cppy::ptr res( ListMethods::pop( m_list.get(), stack, nargs ) );
#else
        cppy::ptr res( ListMethods::pop( m_list.get(), args ) );
#endif
        if( !res )
            return 0;
        if( observer_check() )
        {
            cppy::ptr c( prepare_change() );
            if( !c )
                return 0;  // LCOV_EXCL_LINE
            if( PyDict_SetItem( c.get(), PySStr::operation(), PySStr::pop() ) != 0 )
                return 0;
            // if the superclass call succeeds, then this is safe.
            Py_ssize_t i = -1;
            if( PyTuple_GET_SIZE( args ) == 1 )
                i = PyLong_AsSsize_t( PyTuple_GET_ITEM( args, 0 ) );
            if( i < 0 )
                i += size;
            cppy::ptr index( PyLong_FromSsize_t( i ) );
            if( PyDict_SetItem( c.get(), PySStr::index(), index.get() ) != 0 )
                return 0;
            if( PyDict_SetItem( c.get(), PySStr::item(), res.get() ) != 0 )
                return 0;
            if( !post_change( c ) )
                return 0;
        }
        return res.release();
    }

    PyObject* remove( PyObject* value )
    {
        cppy::ptr res( ListMethods::remove( m_list.get(), value ) );
        if( !res )
            return 0;
        if( observer_check() )
        {
            cppy::ptr c( prepare_change() );
            if( !c )
                return 0;  // LCOV_EXCL_LINE
            if( PyDict_SetItem( c.get(), PySStr::operation(), PySStr::remove() ) != 0)
                return 0;
            if( PyDict_SetItem( c.get(), PySStr::item(), value ) != 0 )
                return 0;
            if( !post_change( c ) )
                return 0;
        }
        return res.release();
    }

    PyObject* reverse()
    {
        int res( PyList_Reverse( m_list.get() ) );
        if( res != 0 )
            return 0;
        if( observer_check() )
        {
            cppy::ptr c( prepare_change() );
            if( !c )
                return 0;  // LCOV_EXCL_LINE
            if( PyDict_SetItem( c.get(), PySStr::operation(), PySStr::reverse() ) != 0)
                return 0;
            if( !post_change( c ) )
                return 0;
        }
        return cppy::incref( Py_None );
    }

    PyObject* sort( PyObject* args, PyObject* kwargs )
    {
        static char *kwlist[] = { "key", "reverse", 0 };
#if PY_VERSION_HEX >= 0x03070000
        int nargs = (int)PyTuple_GET_SIZE( args );
        PyObject **stack = &PyTuple_GET_ITEM( args, 0 );

        PyObject *const *stackbis;
        PyObject *kwnames;
        if (_PyStack_UnpackDict(stack, nargs, kwargs, &stackbis, &kwnames) < 0) {
            return 0;
        }

        cppy::ptr res( ListMethods::sort( m_list.get(), stackbis, nargs, kwnames ) );
        if (stackbis != stack) {
            PyMem_Free((PyObject **)stackbis);
        }
        Py_XDECREF(kwnames);
#else
        cppy::ptr res( ListMethods::sort( m_list.get(), args, kwargs ) );
#endif
        if( !res )
            return 0;
        if( observer_check() )
        {
            cppy::ptr c( prepare_change() );
            if( !c )
                return 0;  // LCOV_EXCL_LINE
            if( PyDict_SetItem( c.get(), PySStr::operation(), PySStr::sort() ) != 0 )
                return 0;
            PyObject* key = Py_None;
            int rev = 0;
            if( !PyArg_ParseTupleAndKeywords(
                args, kwargs, "|Oi", kwlist, &key, &rev ) )
                return 0;
            if( PyDict_SetItem( c.get(), PySStr::key(), key ) != 0)
                return 0;
            if( PyDict_SetItem( c.get(), PySStr::reverse(), rev ? Py_True : Py_False ) != 0 )
                return 0;
            if( !post_change( c ) )
                return 0;
        }
        return res.release();
    }

    PyObject* iadd( PyObject* value )
    {
        cppy::ptr res( AtomListHandler::iadd( value ) );
        if( !res )
            return 0;
        if( observer_check() )
        {
            cppy::ptr c( prepare_change() );
            if( !c )
                return 0;  // LCOV_EXCL_LINE
            if( PyDict_SetItem( c.get(), PySStr::operation(), PySStr::__iadd__() ) != 0 )
                return 0;
            if( PyDict_SetItem( c.get(), PySStr::items(), m_validated.get() ) != 0 )
                return 0;
            if( !post_change( c ) )
                return 0;
        }
        return res.release();
    }

    PyObject* imul( Py_ssize_t count )
    {
        cppy::ptr res( PyList_Type.tp_as_sequence->sq_inplace_repeat(
            m_list.get(), count ) );
        if( !res )
            return 0;
        if( observer_check() )
        {
            cppy::ptr c( prepare_change() );
            if( !c )
                return 0;  // LCOV_EXCL_LINE
            if( PyDict_SetItem( c.get(), PySStr::operation(), PySStr::__imul__() ) != 0 )
                return 0;
            cppy::ptr pycount( PyLong_FromSsize_t( count ) );
            if( !pycount )
                return 0;
            if( PyDict_SetItem( c.get(), PySStr::count(), pycount.get() ) != 0 )
                return 0;
            if( !post_change( c ) )
                return 0;
        }
        return res.release();
    }

    int setitem( Py_ssize_t index, PyObject* value )
    {
        cppy::ptr olditem;
        bool obs = observer_check();
        if( obs )
        {
            olditem = PyList_GetItem( m_list.get(), index );
            if( !olditem )
                return -1;
        }
        int res = AtomListHandler::setitem( index, value );
        if( res < 0 )
            return res;
        if( obs )
        {
            cppy::ptr pyindex( PyLong_FromSsize_t( index ) );
            if( !pyindex )
                return -1;
            res = post_setitem_change( pyindex, olditem, m_validated );
        }
        return res;
    }

    int setitem( PyObject* key, PyObject* value )
    {
        cppy::ptr olditem;
        bool obs = observer_check();
        if( obs )
        {
            olditem = PyObject_GetItem( m_list.get(), key );
            if( !olditem )
                return -1;
        }
        int res = AtomListHandler::setitem( key, value );
        if( res < 0 )
            return res;
        if( obs )
        {
            cppy::ptr index( cppy::incref( key ) );
            res = post_setitem_change( index, olditem, m_validated );
        }
        return res;
    }

private:

    AtomCListHandler();

    AtomCList* clist()
    {
        return atomclist_cast( m_list.get() );
    }

    Member* member()
    {
        return clist()->member;
    }

    bool observer_check()
    {
        m_obsm = false;
        m_obsa = false;
        if( !member() || !atom() )
            return false;
        m_obsm = member()->has_observers();
        m_obsa = atom()->has_observers( member()->name );
        return m_obsm || m_obsa;
    }

    PyObject* prepare_change()
    {
        cppy::ptr c( PyDict_New() );
        if( !c )
            return 0;
        if( PyDict_SetItem( c.get(), PySStr::type(), PySStr::container() ) != 0 )
            return 0;
        if( PyDict_SetItem( c.get(), PySStr::name(), member()->name ) != 0 )
            return 0;
        if( PyDict_SetItem( c.get(), PySStr::object(), pyobject_cast( atom() ) ) != 0 )
            return 0;
        if( PyDict_SetItem( c.get(), PySStr::value(), m_list.get() ) != 0 )
            return 0;
        return c.release();
    }

    bool post_change( cppy::ptr& change )
    {
        cppy::ptr args( PyTuple_New( 1 ) );
        if( !args )
            return false;
        PyTuple_SET_ITEM( args.get(), 0, change.release() );
        if( m_obsm )
        {
            if( !member()->notify( atom(), args.get(), 0 ) )
                return false;
        }
        if( m_obsa )
        {
            if( !atom()->notify( member()->name, args.get(), 0 ) )
                return false;
        }
        return true;
    }

    int post_setitem_change( cppy::ptr& i, cppy::ptr& o, cppy::ptr& n )
    {
        cppy::ptr c( prepare_change() );
        if( !c )
            return -1;
        if( n )
        {
            if( PyDict_SetItem( c.get(), PySStr::operation(), PySStr::__setitem__() ) != 0 )
                return -1;
            if( PyDict_SetItem( c.get(), PySStr::olditem(), o.get() ) != 0)
                return -1;
            if( PyDict_SetItem( c.get(), PySStr::newitem(), n.get() ) != 0)
                return -1;
        }
        else
        {
            if( PyDict_SetItem( c.get(), PySStr::operation(), PySStr::__delitem__() ) != 0 )
                return -1;
            if( PyDict_SetItem( c.get(), PySStr::item(), o.get() ) != 0 )
                return -1;
        }
        if( PyDict_SetItem( c.get(), PySStr::index(), i.get() ) != 0 )
            return -1;
        if( !post_change( c ) )
            return -1;
        return 0;
    }

    bool m_obsm;
    bool m_obsa;
};

}  // namespace


static PyObject*
AtomCList_new( PyTypeObject* type, PyObject* args, PyObject* kwargs )
{
    return AtomList_Type.tp_new( type, args, kwargs );
}


int AtomCList_clear( AtomCList* self )
{
    Py_CLEAR( self->member );
    return AtomList_clear( atomlist_cast( self )  );
}


int AtomCList_traverse( AtomCList* self, visitproc visit, void* arg )
{
    Py_VISIT( self->member );
    return AtomList_traverse( atomlist_cast( self ) , visit, arg );
}


static void
AtomCList_dealloc( AtomCList* self )
{
    PyObject_GC_UnTrack( self );
    Py_CLEAR( self->member );
    delete atomlist_cast( self )->pointer;
    atomlist_cast( self )->pointer = 0;
    Py_CLEAR( atomlist_cast( self )->validator );
    PyList_Type.tp_dealloc( pyobject_cast( self ) );
}


static PyObject*
AtomCList_append( AtomCList* self, PyObject* value )
{
    return AtomCListHandler( self ).append( value );
}


static PyObject*
AtomCList_insert( AtomCList* self, PyObject* args )
{
    return AtomCListHandler( self ).insert( args );
}


static PyObject*
AtomCList_extend( AtomCList* self, PyObject* value )
{
    return AtomCListHandler( self ).extend( value );
}


static PyObject*
AtomCList_pop( AtomCList* self, PyObject* args )
{
    return AtomCListHandler( self ).pop( args );
}


static PyObject*
AtomCList_remove( AtomCList* self, PyObject* value )
{
    return AtomCListHandler( self ).remove( value );
}


static PyObject*
AtomCList_reverse( AtomCList* self )
{
    return AtomCListHandler( self ).reverse();
}


static PyObject*
AtomCList_sort( AtomCList* self, PyObject* args, PyObject* kwargs )
{
    return AtomCListHandler( self ).sort( args, kwargs );
}


static int
AtomCList_ass_item( AtomCList* self, Py_ssize_t index, PyObject* value )
{
    return AtomCListHandler( self ).setitem( index, value );
}


static PyObject*
AtomCList_inplace_concat( AtomCList* self, PyObject* value )
{
    return AtomCListHandler( self ).iadd( value );
}


static PyObject*
AtomCList_inplace_repeat( AtomCList* self, Py_ssize_t count )
{
    return AtomCListHandler( self ).imul( count );
}


static int
AtomCList_ass_subscript( AtomCList* self, PyObject* key, PyObject* value )
{
    return AtomCListHandler( self ).setitem( key, value );
}


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


static PySequenceMethods
AtomCList_as_sequence = {
    ( lenfunc )0,                                 /* sq_length */
    ( binaryfunc )0,                              /* sq_concat */
    ( ssizeargfunc )0,                            /* sq_repeat */
    ( ssizeargfunc )0,                            /* sq_item */
    ( void * )0,                                  /* sq_slice */
    ( ssizeobjargproc )AtomCList_ass_item,        /* sq_ass_item */
    ( void * )0,                                  /* sq_ass_slice */
    ( objobjproc )0,                              /* sq_contains */
    ( binaryfunc )AtomCList_inplace_concat,       /* sq_inplace_concat */
    ( ssizeargfunc )AtomCList_inplace_repeat,     /* sq_inplace_repeat */
};


static PyMappingMethods
AtomCList_as_mapping = {
    ( lenfunc )0,                             /* mp_length */
    ( binaryfunc )0,                          /* mp_subscript */
    ( objobjargproc )AtomCList_ass_subscript  /* mp_ass_subscript */
};


PyTypeObject AtomCList_Type = {
    PyVarObject_HEAD_INIT( &PyType_Type, 0 )
    PACKAGE_TYPENAME( "atomclist" ),              /* tp_name */
    sizeof( AtomCList ),                          /* tp_basicsize */
    0,                                            /* tp_itemsize */
    ( destructor )AtomCList_dealloc,              /* tp_dealloc */
    ( printfunc )0,                               /* tp_print */
    ( getattrfunc )0,                             /* tp_getattr */
    ( setattrfunc )0,                             /* tp_setattr */
	( PyAsyncMethods* )0,                         /* tp_as_async */
    ( reprfunc )0,                                /* tp_repr */
    ( PyNumberMethods* )0,                        /* tp_as_number */
    ( PySequenceMethods* )&AtomCList_as_sequence, /* tp_as_sequence */
    ( PyMappingMethods* )&AtomCList_as_mapping,   /* tp_as_mapping */
    ( hashfunc )0,                                /* tp_hash */
    ( ternaryfunc )0,                             /* tp_call */
    ( reprfunc )0,                                /* tp_str */
    ( getattrofunc )0,                            /* tp_getattro */
    ( setattrofunc )0,                            /* tp_setattro */
    ( PyBufferProcs* )0,                          /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT
    |Py_TPFLAGS_BASETYPE
    |Py_TPFLAGS_HAVE_GC,                          /* tp_flags */
    0,                                            /* Documentation string */
    ( traverseproc )AtomCList_traverse,           /* tp_traverse */
    ( inquiry )AtomCList_clear,                   /* tp_clear */
    ( richcmpfunc )0,                             /* tp_richcompare */
    0,                                            /* tp_weaklistoffset */
    ( getiterfunc )0,                             /* tp_iter */
    ( iternextfunc )0,                            /* tp_iternext */
    ( struct PyMethodDef* )AtomCList_methods,     /* tp_methods */
    ( struct PyMemberDef* )0,                     /* tp_members */
    0,                                            /* tp_getset */
    &AtomList_Type,                               /* tp_base */
    0,                                            /* tp_dict */
    ( descrgetfunc )0,                            /* tp_descr_get */
    ( descrsetfunc )0,                            /* tp_descr_set */
    0,                                            /* tp_dictoffset */
    ( initproc )0,                                /* tp_init */
    ( allocfunc )0,                               /* tp_alloc */
    ( newfunc )AtomCList_new,                     /* tp_new */
    ( freefunc )0,                                /* tp_free */
    ( inquiry )0,                                 /* tp_is_gc */
    0,                                            /* tp_bases */
    0,                                            /* tp_mro */
    0,                                            /* tp_cache */
    0,                                            /* tp_subclasses */
    0,                                            /* tp_weaklist */
    ( destructor )0                               /* tp_del */
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
