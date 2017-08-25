/*-----------------------------------------------------------------------------
| Copyright (c) 2013, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include "atomlist.h"
#include "packagenaming.h"


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
    if( size < 0 )
        return py_bad_internal_call( "negative list size" );
    if( static_cast<size_t>( size ) > PY_SIZE_MAX / sizeof( PyObject* ) )
        return PyErr_NoMemory();
    PyObjectPtr ptr( PyType_GenericNew( subtype, 0, 0 ) );
    if( !ptr )
        return 0;
    PyListObject* op = reinterpret_cast<PyListObject*>( ptr.get() );
    if( size > 0 )
    {
        size_t nbytes = size * sizeof( PyObject* );
        op->ob_item = reinterpret_cast<PyObject**>( PyMem_Malloc( nbytes ) );
        if( !op->ob_item )
            return PyErr_NoMemory();
        memset( op->ob_item, 0, nbytes );
    }
    Py_SIZE( op ) = size;
    op->allocated = size;
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
namespace
{

class AtomListHandler
{

public:

    AtomListHandler( AtomList* list ) : 
        m_list( newref( pyobject_cast( list ) ) ) {}

    PyObject* append( PyObject* value )
    {
        PyObjectPtr item( validate_single( value ) );
        if( !item )
            return 0;
        return ListMethods::append( m_list.get(), item.get() );
    }

    PyObject* insert( PyObject* args )
    {
        Py_ssize_t index;
        PyObject* value;
        if( !PyArg_ParseTuple( args, "nO:insert", &index, &value ) )
            return 0;
        PyObjectPtr valptr( validate_single( value ) );
        if( !valptr )
            return 0;
        PyTuplePtr nargs( PyTuple_New( 2 ) );
        if( !nargs )
            return 0;
        nargs.initialize( 0, PyLong_FromSsize_t( index ) );
        nargs.initialize( 1, valptr.release() );
        return ListMethods::insert( m_list.get(), nargs.get() );
    }

    PyObject* extend( PyObject* value )
    {
        PyObjectPtr item( validate_sequence( value ) );
        if( !item )
            return 0;
        return ListMethods::extend( m_list.get(), item.get() );
    }

    PyObject* iadd( PyObject* value )
    {
        PyObjectPtr item( validate_sequence( value ) );
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
        PyObjectPtr item( validate_single( value ) );
        if( !item )
            return -1;
        return PyList_Type.tp_as_sequence->sq_ass_item(
            m_list.get(), index, item.get() );
    }

    int setitem( Py_ssize_t low, Py_ssize_t high, PyObject* value )
    {
        #if PY_MAJOR_VERSION >= 3
            //if ( !value )
            //    return PyList_Type.tp_as_mapping->mp_
            // TODO - !value check here
            int i, size = PySequence_Size( value );
            if (size < 0)
                return -1;
            for ( i=0; i<size; i++ )
            {
                int loc = i + (int)low;
                PyObject * obj = PySequence_GetItem( value, i );
                PyObjectPtr item( validate_single( obj ) );
                PyList_Type.tp_as_sequence->sq_ass_item(
                    m_list.get(), loc, item.get());
                Py_DECREF( obj );
            }
            return 1;
        #else
            if( !value )
                return PyList_Type.tp_as_sequence->sq_ass_slice(
                    m_list.get(), low, high, value );
            PyObjectPtr item( validate_sequence( value ) );
            if( !item )
                return -1;
            return PyList_Type.tp_as_sequence->sq_ass_slice(
                m_list.get(), low, high, item.get() );
        #endif
    }

    int setitem( PyObject* key, PyObject* value )
    {
        if( !value )
            return PyList_Type.tp_as_mapping->mp_ass_subscript(
                m_list.get(), key, value );
        PyObjectPtr item;
        if( PyIndex_Check( key ) )
            item = validate_single( value );
        else if( PySlice_Check( key ) )
            item = validate_sequence( value );
        else
            item = newref( value );
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
        PyObjectPtr item( newref( value ) );
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
        PyObjectPtr item( newref( value ) );
        if( validator() && atom() )
        {
            // no validation needed for self[::-1] = self
            if( m_list.get() != value )
            {
                PyListPtr templist( PySequence_List( value ) );
                if( !templist )
                    return 0;
                CAtom* atm = atom();
                Member* vd = validator();
                Py_ssize_t size = templist.size();
                for( Py_ssize_t i = 0; i < size; ++i )
                {
                    PyObject* b = templist.borrow_item( i );
                    PyObject* val = vd->full_validate( atm, Py_None, b );
                    if( !val )
                        return 0;
                    templist.set_item( i, val );
                }
                item = templist;
            }
        }
        m_validated = item;
        return item.release();
    }

    PyListPtr m_list;
    PyObjectPtr m_validated;

private:

    AtomListHandler();
};

}  // namespace


static PyObject*
AtomList_new( PyTypeObject* type, PyObject* args, PyObject* kwargs )
{
    PyObjectPtr ptr( PyList_Type.tp_new( type, args, kwargs ) );
    if( !ptr )
        return 0;
    atomlist_cast( ptr.get() )->pointer = new CAtomPointer();
    return ptr.release();
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
    return AtomListHandler( self ).setitem( index, value );
}

static int
AtomList_ass_slice( AtomList* self, Py_ssize_t low, Py_ssize_t high, PyObject* value )
{
    return AtomListHandler( self ).setitem( low, high, value );
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
    (lenfunc)0,                                 /* sq_length */
    (binaryfunc)0,                              /* sq_concat */
    (ssizeargfunc)0,                            /* sq_repeat */
    (ssizeargfunc)0,                            /* sq_item */
    #if PY_MAJOR_VERSION >= 3
        (void*)0,                                   /* sq_slice */
    #else
        (ssizessizeargfunc)0,                       /* sq_slice */
    #endif
    (ssizeobjargproc)AtomList_ass_item,         /* sq_ass_item */
    #if PY_MAJOR_VERSION >= 3
        (void*)0,                                   /* sq_ass_slice */
    #else
        (ssizessizeobjargproc)AtomList_ass_slice,   /* sq_ass_slice */
    #endif
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
    PyVarObject_HEAD_INIT( &PyType_Type, 0 )
    //0,                                      /* ob_size */
    PACKAGE_TYPENAME( "atomlist" ),         /* tp_name */
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
        m_pystring = PyUnicode_FromString( string );
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

public:

    AtomCListHandler( AtomCList* list ) :
        AtomListHandler( atomlist_cast( list ) ),
        m_obsm( false ), m_obsa( false ) {}

    PyObject* append( PyObject* value )
    {
        PyObjectPtr res( AtomListHandler::append( value ) );
        if( !res )
            return 0;
        if( observer_check() )
        {
            PyDictPtr c( prepare_change() );
            if( !c )
                return 0;
            if( !c.set_item( PySStr::operation(), PySStr::append() ) )
                return 0;
            if( !c.set_item( PySStr::item(), m_validated ) )
                return 0;
            if( !post_change( c ) )
                return 0;
        }
        return res.release();
    }

    PyObject* insert( PyObject* args )
    {
        Py_ssize_t size = m_list.size();
        PyObjectPtr res( AtomListHandler::insert( args ) );
        if( !res )
            return 0;
        if( observer_check() )
        {
            PyDictPtr c( prepare_change() );
            if( !c )
                return 0;
            if( !c.set_item( PySStr::operation(), PySStr::insert() ) )
                return 0;
            // if the superclass call succeeds, then this is safe.
            Py_ssize_t where = PyLong_AsSsize_t( PyTuple_GET_ITEM( args, 0 ) );
            clip_index( where, size );
            PyObjectPtr index( PyLong_FromSsize_t( where ) );
            if( !c.set_item( PySStr::index(), index ) )
                return 0;
            if( !c.set_item( PySStr::item(), m_validated ) )
                return 0;
            if( !post_change( c ) )
                return 0;
        }
        return res.release();
    }

    PyObject* extend( PyObject* value )
    {
        PyObjectPtr res( AtomListHandler::extend( value ) );
        if( !res )
            return 0;
        if( observer_check() )
        {
            PyDictPtr c( prepare_change() );
            if( !c )
                return 0;
            if( !c.set_item( PySStr::operation(), PySStr::extend() ) )
                return 0;
            if( !c.set_item( PySStr::items(), m_validated ) )
                return 0;
            if( !post_change( c ) )
                return 0;
        }
        return res.release();
    }

    PyObject* pop( PyObject* args )
    {
        Py_ssize_t size = m_list.size();
        PyObjectPtr res( ListMethods::pop( m_list.get(), args ) );
        if( !res )
            return 0;
        if( observer_check() )
        {
            PyDictPtr c( prepare_change() );
            if( !c )
                return 0;
            if( !c.set_item( PySStr::operation(), PySStr::pop() ) )
                return 0;
            // if the superclass call succeeds, then this is safe.
            Py_ssize_t i = -1;
            if( PyTuple_GET_SIZE( args ) == 1 )
                i = PyLong_AsSsize_t( PyTuple_GET_ITEM( args, 0 ) );
            if( i < 0 )
                i += size;
            PyObjectPtr index( PyLong_FromSsize_t( i ) );
            if( !c.set_item( PySStr::index(), index ) )
                return 0;
            if( !c.set_item( PySStr::item(), res ) )
                return 0;
            if( !post_change( c ) )
                return 0;
        }
        return res.release();
    }

    PyObject* remove( PyObject* value )
    {
        PyObjectPtr res( ListMethods::remove( m_list.get(), value ) );
        if( !res )
            return 0;
        if( observer_check() )
        {
            PyDictPtr c( prepare_change() );
            if( !c )
                return 0;
            if( !c.set_item( PySStr::operation(), PySStr::remove() ) )
                return 0;
            if( !c.set_item( PySStr::item(), value ) )
                return 0;
            if( !post_change( c ) )
                return 0;
        }
        return res.release();
    }

    PyObject* reverse()
    {
        PyObjectPtr res( ListMethods::reverse( m_list.get(), 0 ) );
        if( !res )
            return 0;
        if( observer_check() )
        {
            PyDictPtr c( prepare_change() );
            if( !c )
                return 0;
            if( !c.set_item( PySStr::operation(), PySStr::reverse() ) )
                return 0;
            if( !post_change( c ) )
                return 0;
        }
        return res.release();
    }

    PyObject* sort( PyObject* args, PyObject* kwargs )
    {
        static char *kwlist[] = { "cmp", "key", "reverse", 0 };
        PyObjectPtr res( ListMethods::sort( m_list.get(), args, kwargs ) );
        if( !res )
            return 0;
        if( observer_check() )
        {
            PyDictPtr c( prepare_change() );
            if( !c )
                return 0;
            if( !c.set_item( PySStr::operation(), PySStr::sort() ) )
                return 0;
            PyObject* cmp = Py_None;
            PyObject* key = Py_None;
            int rev = 0;
            if( !PyArg_ParseTupleAndKeywords(
                args, kwargs, "|OOi", kwlist, &cmp, &key, &rev ) )
                return 0;
            if( !c.set_item( PySStr::cmp(), cmp ) )
                return 0;
            if( !c.set_item( PySStr::key(), key ) )
                return 0;
            if( !c.set_item( PySStr::reverse(), rev ? Py_True : Py_False ) )
                return 0;
            if( !post_change( c ) )
                return 0;
        }
        return res.release();
    }

    PyObject* iadd( PyObject* value )
    {
        PyObjectPtr res( AtomListHandler::iadd( value ) );
        if( !res )
            return 0;
        if( observer_check() )
        {
            PyDictPtr c( prepare_change() );
            if( !c )
                return 0;
            if( !c.set_item( PySStr::operation(), PySStr::__iadd__() ) )
                return 0;
            if( !c.set_item( PySStr::items(), m_validated ) )
                return 0;
            if( !post_change( c ) )
                return 0;
        }
        return res.release();
    }

    PyObject* imul( Py_ssize_t count )
    {
        PyObjectPtr res( PyList_Type.tp_as_sequence->sq_inplace_repeat(
            m_list.get(), count ) );
        if( !res )
            return 0;
        if( observer_check() )
        {
            PyDictPtr c( prepare_change() );
            if( !c )
                return 0;
            if( !c.set_item( PySStr::operation(), PySStr::__imul__() ) )
                return 0;
            PyObjectPtr pycount( PyLong_FromSsize_t( count ) );
            if( !pycount )
                return 0;
            if( !c.set_item( PySStr::count(), pycount ) )
                return 0;
            if( !post_change( c ) )
                return 0;
        }
        return res.release();
    }

    int setitem( Py_ssize_t index, PyObject* value )
    {
        PyObjectPtr olditem;
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
            PyObjectPtr pyindex( PyLong_FromSsize_t( index ) );
            if( !pyindex )
                return -1;
            res = post_setitem_change( pyindex, olditem, m_validated );
        }
        return res;
    }

    int setitem( Py_ssize_t low, Py_ssize_t high, PyObject* value )
    {
        PyObjectPtr olditem;
        bool obs = observer_check();
        if( obs )
        {
            olditem = PyList_GetSlice( m_list.get(), low, high );
            if( !olditem )
                return -1;
        }
        int res = AtomListHandler::setitem( low, high, value );
        if( res < 0 )
            return res;
        if( obs )
        {
            PyObjectPtr index( _PySlice_FromIndices( low, high ) );
            if( !index )
                return -1;
            res = post_setitem_change( index, olditem, m_validated );
        }
        return res;
    }

    int setitem( PyObject* key, PyObject* value )
    {
        PyObjectPtr olditem;
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
            PyObjectPtr index( newref( key ) );
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
        PyDictPtr c( PyDict_New() );
        if( !c )
            return 0;
        if( !c.set_item( PySStr::type(), PySStr::container() ) )
            return 0;
        if( !c.set_item( PySStr::name(), member()->name ) )
            return 0;
        if( !c.set_item( PySStr::object(), pyobject_cast( atom() ) ) )
            return 0;
        if( !c.set_item( PySStr::value(), m_list.get() ) )
            return 0;
        return c.release();
    }

    bool post_change( PyObjectPtr& change )
    {
        PyTuplePtr args( PyTuple_New( 1 ) );
        if( !args )
            return false;
        args.set_item( 0, change );
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

    int post_setitem_change( PyObjectPtr& i, PyObjectPtr& o, PyObjectPtr& n )
    {
        PyDictPtr c( prepare_change() );
        if( !c )
            return -1;
        if( n )
        {
            if( !c.set_item( PySStr::operation(), PySStr::__setitem__() ) )
                return -1;
            if( !c.set_item( PySStr::olditem(), o ) )
                return -1;
            if( !c.set_item( PySStr::newitem(), n ) )
                return -1;
        }
        else
        {
            if( !c.set_item( PySStr::operation(), PySStr::__delitem__() ) )
                return -1;
            if( !c.set_item( PySStr::item(), o ) )
                return -1;
        }
        if( !c.set_item( PySStr::index(), i ) )
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


static void
AtomCList_dealloc( AtomCList* self )
{
    Py_CLEAR( self->member );
    AtomList_dealloc( atomlist_cast( self ) );
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


static int
AtomCList_ass_slice( AtomCList* self, Py_ssize_t low, Py_ssize_t high, PyObject* value )
{
    return AtomCListHandler( self ).setitem( low, high, value );
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
    (lenfunc)0,                                 /* sq_length */
    (binaryfunc)0,                              /* sq_concat */
    (ssizeargfunc)0,                            /* sq_repeat */
    (ssizeargfunc)0,                            /* sq_item */
    #if PY_MAJOR_VERSION >= 3
        (void*)0,
    #else
        (ssizessizeargfunc)0,                       /* sq_slice */
    #endif
    (ssizeobjargproc)AtomCList_ass_item,        /* sq_ass_item */
    #if PY_MAJOR_VERSION >= 3
        (void*)0,                               /* sq_ass_slice (python3) */
    #else
        (ssizessizeobjargproc)AtomCList_ass_slice,  /* sq_ass_slice */
    #endif
    (objobjproc)0,                              /* sq_contains */
    (binaryfunc)AtomCList_inplace_concat,       /* sq_inplace_concat */
    (ssizeargfunc)AtomCList_inplace_repeat,     /* sq_inplace_repeat */
};


static PyMappingMethods
AtomCList_as_mapping = {
    (lenfunc)0,                             /* mp_length */
    (binaryfunc)0,                          /* mp_subscript */
    (objobjargproc)AtomCList_ass_subscript  /* mp_ass_subscript */
};


PyTypeObject AtomCList_Type = {
    PyVarObject_HEAD_INIT( &PyType_Type, 0 )
    //0,                                     /* ob_size */
    PACKAGE_TYPENAME( "atomclist" ),        /* tp_name */
    sizeof( AtomCList ),                    /* tp_basicsize */
    0,                                      /* tp_itemsize */
    (destructor)AtomCList_dealloc,          /* tp_dealloc */
    (printfunc)0,                           /* tp_print */
    (getattrfunc)0,                         /* tp_getattr */
    (setattrfunc)0,                         /* tp_setattr */
    (cmpfunc)0,                             /* tp_compare */
    (reprfunc)0,                            /* tp_repr */
    (PyNumberMethods*)0,                    /* tp_as_number */
    (PySequenceMethods*)&AtomCList_as_sequence, /* tp_as_sequence */
    (PyMappingMethods*)&AtomCList_as_mapping,   /* tp_as_mapping */
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
