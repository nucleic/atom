/*-----------------------------------------------------------------------------
| Copyright (c) 2013, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include "atomdict.h"
#include "packagenaming.h"


using namespace PythonHelpers;


namespace DictMethods
{

static PyCFunction popitem = 0;
static PyCFunction clear = 0;
static PyCFunctionWithKeywords pop = 0;
static PyCFunctionWithKeywords setdefault = 0;
static PyCFunctionWithKeywords update = 0;


static bool
init_methods()
{
    popitem = lookup_method( &PyDict_Type, "popitem" );
    if( !popitem )
    {
        py_bad_internal_call( "failed to load dict 'popitem' method" );
        return false;
    }
    clear = lookup_method( &PyDict_Type, "clear" );
    if( !clear )
    {
        py_bad_internal_call( "failed to load dict 'clear' method" );
        return false;
    }
    typedef PyCFunctionWithKeywords func_t;
    setdefault = reinterpret_cast<func_t>( lookup_method( &PyDict_Type, "setdefault" ) );
    if( !setdefault )
    {
        py_bad_internal_call( "failed to load dict 'setdefault' method" );
        return false;
    }
    pop = reinterpret_cast<func_t>( lookup_method( &PyDict_Type, "pop" ) );
    if( !pop )
    {
        py_bad_internal_call( "failed to load dict 'poo' method" );
        return false;
    }
    update = reinterpret_cast<func_t>( lookup_method( &PyDict_Type, "update" ) );
    if( !update )
    {
        py_bad_internal_call( "failed to load dict 'update' method" );
        return false;
    }
    return true;
}

}  // namespace DictMethods


static PyObject*
DictSubtype_New( PyTypeObject* subtype )
{
    PyObjectPtr ptr( PyType_GenericNew( subtype, 0, 0 ) );
    if( !ptr )
        return 0;
    PyDictObject* op = reinterpret_cast<PyDictObject*>( ptr.get() );
    return ptr.release();
}


PyObject*
AtomDict_New( CAtom* atom, Member* key_validator, Member* value_validator )
{
    PyObjectPtr ptr( DictSubtype_New( &AtomDict_Type ) );
    if( !ptr )
        return 0;
    Py_XINCREF( pyobject_cast( key_validator ) );
    Py_XINCREF( pyobject_cast( value_validator ) );
    atomdict_cast( ptr.get() )->key_validator = key_validator;
    atomdict_cast( ptr.get() )->value_validator = value_validator;
    atomdict_cast( ptr.get() )->pointer = new CAtomPointer( atom );
    return ptr.release();
}


PyObject*
AtomCDict_New( CAtom* atom, Member* validator, Member* member )
{
    PyObjectPtr ptr( DictSubtype_New( &AtomCDict_Type ) );
    if( !ptr )
        return 0;
    Py_XINCREF( pyobject_cast( key_validator ) );
    Py_XINCREF( pyobject_cast( value_validator ) );
    Py_XINCREF( pyobject_cast( member ) );
    atomdict_cast( ptr.get() )->key_validator = key_validator;
    atomdict_cast( ptr.get() )->value_validator = value_validator;
    atomcdict_cast( ptr.get() )->pointer = new CAtomPointer( atom );
    atomcdict_cast( ptr.get() )->member = member;
    return ptr.release();
}

/*-----------------------------------------------------------------------------
| AtomDict Type
|----------------------------------------------------------------------------*/
namespace
{

class AtomDictHandler
{

public:

    AtomDictHandler( AtomDict* dict ) :
        m_dict( newref( pyobject_cast( dict ) ) ) {}

    /*PyObject* setdefault( PyObject* key, PyObject* kwargs )
    {
        PyObject
        PyObjectPtr item( validate_single( value ) );
        if( !item )
            return 0;
        return ListMethods::append( m_list.get(), item.get() );
    }*/

    PyObject* update( PyObject* value )
    {
        if( PyDict_Check( value ) )
            PyObjectPtr item( validate_dict( value ) );
        else
            PyObjectPtr item( validate_pairs( value ) );
        if( !item )
            return 0;
        return DictMethods::update( m_list.get(), item.get() );
    }

    int setitem( PyObject* key, PyObject* value )
    {
        if( !value )
            return PyDict_Type.tp_as_mapping->mp_ass_subscript(
                m_dict.get(), key, value );
        PyObjectPtr item;
        item = validate_key_value_pair( key, value );
        if( !item )
            return -1;
        PyObjectPtr pair = PyList_GET_ITEM(PyDict_Items(item), 0);
        return PyDict_Type.tp_as_mapping->mp_ass_subscript(
            m_list.get(), PyList_GET_ITEM(pair, 0), PyList_GET_ITEM(pair, 1) );
    }

protected:

    AtomList* adict()
    {
        return atomdict_cast( m_dict.get() );
    }

    Member* key_validator()
    {
        return adict()->key_validator;
    }

    Member* value_validator()
    {
        return adict()->value_validator;
    }

    CAtom* atom()
    {
        return adict()->pointer->data();
    }

    PyObject* validate_key_value_pair( PyObject* key , PyObject* value )
    {
        PyDictPtr item( PyDict_New() );
        if( key_validator() && atom() )
        {
            PyObject* val_key = key_validator()->full_validate( atom(), Py_None, key );
            if( !val_key )
                return 0;
        }
        if( value_validator() && atom() )
        {
            PyObject* val_val = value_validator()->full_validate( atom(), Py_None, value );
            if( !val_val )
                return 0;
        }
        item.set_item(val_key, val_val)
        m_validated = item;
        return item.release();
    }

    PyObject* validate_pairs( PyObject* value )
    {
        PyObjectPtr item( newref( value ) );
        if( validator() && atom() )
        {

            PyListPtr temppairs( PySequence_List( value ));
            PyDictPtr tempdict( PyDict_New() );
            if( !temppairs )
                return 0;
            CAtom* atm = atom();
            Member* key_vd = key_validator();
            Member* value_vd = value_validator();
            Py_ssize_t size = temppairs.size();
            for( Py_ssize_t i = 0; i < size; ++i )
            {
                PyListPtr pair = temppairs.borrow_item( i );
                PyObject* key = pair.borrow_item( 0 );
                PyObject* val_key = key_vd->full_validate( atm, Py_None, key );
                if( !val_key )
                    return 0;
                PyObject* val = pair.borrow_item( 1 );
                PyObject* val_val = val_vd->full_validate( atm, Py_None, val);
                if( !val_val )
                    return 0;
                tempdict.set_item(val_key, val_val);
            }
            item = tempdict;

        }
        m_validated = item;
        return item.release();
    }

    PyObject* validate_dict( PyObject* value )
    {
        PyObjectPtr item( newref( value ) );
        if( validator() && atom() )
        {
            // no validation if the dict is identic to the one we have.
            if( m_dict.get() != value )
            {
                PyDictPtr tempdict( PyDict_New() );
                PyListPtr pairs( PyDict_Items( value ));
                if( !pairs )
                    return 0;
                CAtom* atm = atom();
                Member* key_vd = key_validator();
                Member* value_vd = value_validator();
                Py_ssize_t size = pairs.size();
                for( Py_ssize_t i = 0; i < size; ++i )
                {
                    PyListPtr pair = pairs.borrow_item( i );
                    PyObject* key = pair.borrow_item( 0 );
                    PyObject* val_key = key_vd->full_validate( atm, Py_None, key );
                    if( !val_key )
                        return 0;
                    PyObject* val = pair.borrow_item( 1 );
                    PyObject* val_val = val_vd->full_validate( atm, Py_None, val);
                    if( !val_val )
                        return 0;
                    tempdict.set_item( val_key, val_val );
                }
                item = tempdict;
            }
        }
        m_validated = item;
        return item.release();
    }

    PyDictPtr m_dict;
    PyObjectPtr m_validated;

private:

    AtomDictHandler();
};

}  // namespace


static PyObject*
AtomDict_new( PyTypeObject* type, PyObject* args, PyObject* kwargs )
{
    PyObjectPtr ptr( PyDict_Type.tp_new( type, args, kwargs ) );
    if( !ptr )
        return 0;
    atomlist_cast( ptr.get() )->pointer = new CAtomPointer();
    return ptr.release();
}


static void
AtomDict_dealloc( AtomDict* self )
{
    delete self->pointer;
    self->pointer = 0;
    Py_CLEAR( self->validator );
    PyDict_Type.tp_dealloc( pyobject_cast( self ) );
}


static PyObject*
AtomDict_update( AtomDict* self, PyObject* value )
{
    return AtomDictHandler( self ).update( value );
}



static PyObject*
AtomDict_reduce_ex( AtomDict* self, PyObject* proto )
{
    // An atomdict is pickled as a normal dict. When the Atom class is
    // reconstituted, assigning the dict to the attribute will create
    // a new atomdict with the proper owner. There is no need to try
    // to persist the validator and pointer information.
    PyObjectPtr data( PyDict_Copy( pyobject_cast( self ) ) );
    if( !data )
        return 0;
    PyTuplePtr res( PyTuple_New( 2 ) );
    if( !res )
        return 0;
    PyTuplePtr args( PyTuple_New( 1 ) );
    if( !args )
        return 0;
    args.set_item( 0, data );
    res.set_item( 0, newref( pyobject_cast( &PyDict_Type ) ) );
    res.set_item( 1, args );
    return res.release();
}


static int
AtomDict_ass_subscript( AtomDict* self, PyObject* key, PyObject* value )
{
    return AtomDictHandler( self ).setitem( key, value );
}


PyDoc_STRVAR( a_update_doc,
"D.update(object) -- update dict content using obj" );


static PyMethodDef
AtomDict_methods[] = {
    { "update", ( PyCFunction )AtomDict_update, METH_VARARGS, a_update_doc },
    { "__reduce_ex__", ( PyCFunction )AtomDict_reduce_ex, METH_O, "" },
    { 0 }  /* sentinel */
};



static PyMappingMethods
AtomDict_as_mapping = {
    (lenfunc)0,                             /* mp_length */
    (binaryfunc)0,                          /* mp_subscript */
    (objobjargproc)AtomDict_ass_subscript   /* mp_ass_subscript */
};


PyTypeObject AtomDict_Type = {
    PyObject_HEAD_INIT( &PyType_Type )
    0,                                      /* ob_size */
    PACKAGE_TYPENAME( "atomdict" ),         /* tp_name */
    sizeof( AtomDict ),                     /* tp_basicsize */
    0,                                      /* tp_itemsize */
    (destructor)AtomDict_dealloc,           /* tp_dealloc */
    (printfunc)0,                           /* tp_print */
    (getattrfunc)0,                         /* tp_getattr */
    (setattrfunc)0,                         /* tp_setattr */
    (cmpfunc)0,                             /* tp_compare */
    (reprfunc)0,                            /* tp_repr */
    (PyNumberMethods*)0,                    /* tp_as_number */
    (PySequenceMethods*)0,                  /* tp_as_sequence */
    (PyMappingMethods*)&AtomDict_as_mapping,   /* tp_as_mapping */
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
    (struct PyMethodDef*)AtomDict_methods,  /* tp_methods */
    (struct PyMemberDef*)0,                 /* tp_members */
    0,                                      /* tp_getset */
    &PyDict_Type,                           /* tp_base */
    0,                                      /* tp_dict */
    (descrgetfunc)0,                        /* tp_descr_get */
    (descrsetfunc)0,                        /* tp_descr_set */
    0,                                      /* tp_dictoffset */
    (initproc)0,                            /* tp_init */
    (allocfunc)0,                           /* tp_alloc */
    (newfunc)AtomDict_new,                  /* tp_new */
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
import_atomdict()
{
    if( PyType_Ready( &AtomDict_Type ) < 0 )
        return -1;
    /*if( PyType_Ready( &AtomCList_Type ) < 0 )
        return -1;*/
    if( !ListMethods::init_methods() )
        return -1;
    return 0;
}