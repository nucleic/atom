/*-----------------------------------------------------------------------------
| Copyright (c) 2013, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include "atomdict.h"
#include "packagenaming.h"
#include <Python.h>


using namespace PythonHelpers;


namespace DictMethods
{

static PyCFunction popitem = 0;
static PyCFunction clear = 0;
static PyCFunctionWithKeywords pop = 0;
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
    PyObjectPtr ptr( subtype->tp_new( subtype, 0, 0) );
    if( !ptr )
        return 0;
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


/*PyObject*
AtomCDict_New( CAtom* atom, Member* key_validator, Member* value_validator, Member* member )
{
    PyObjectPtr ptr( DictSubtype_New( &AtomCDict_Type ) );
    if( !ptr )
        return 0;
    Py_XINCREF( pyobject_cast( key_validator ) );
    Py_XINCREF( pyobject_cast( value_validator ) );
    Py_XINCREF( pyobject_cast( member ) );
    atomdict_cast( ptr.get() )->key_validator = key_validator;
    atomdict_cast( ptr.get() )->value_validator = value_validator;
    atomdict_cast( ptr.get() )->pointer = new CAtomPointer( atom );
    atomcdict_cast( ptr.get() )->member = member;
    return ptr.release();
}
*/
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

    PyObject* setdefault( PyObject* args )
    {
        PyObject* key;
        PyObject* default_val = Py_None;
        if( !PyArg_ParseTuple( args, "O|O:setdefault", &key, &default_val ) )
            return 0;


        PyObjectPtr value;
        if( PyDict_Contains( m_dict.get(), key ) )
            value =  m_dict.get_item( key );

        else
        {
            PyTuplePtr item( validate_key_value_pair( key, default_val ) );
            if( !item )
                return 0;

            PyObjectPtr val_key( item.get_item( 0 ) );
            value = item.get_item( 1 );
            if( !m_dict.set_item( val_key, value ) )
                return 0;
            
        }
        if( !value )
            return 0;
        return value.release();
    }

    PyObject* update( PyObject* args, PyObject* kwargs )
    {
        PyObjectPtr item;
        PyObject* value;
        if( PyDict_Size(kwargs) > 0)
            value = kwargs;
        else
            if( !PyArg_ParseTuple( args,  "O:dict", &value ) )
                return 0;

        if( PyDict_Check( value ) )
            item =  validate_dict( value );
        else
            item = validate_pairs( value );

        if( !item )
            return 0;
        
        PyTuplePtr new_args( PyTuple_New( 1 ) );
        new_args.initialize( 0, item );
        return DictMethods::update( m_dict.get(), new_args.get(), 0 );
    }

    int setitem( PyObject* key, PyObject* value )
    {
        PyObjectPtr val_key;
        if( !value )
        {
            val_key = validate_key( key );
            if( !val_key )
                return -1;

            return PyDict_Type.tp_as_mapping->mp_ass_subscript(
                m_dict.get(), val_key.get(), value );
        }

        PyTuplePtr item( validate_key_value_pair( key, value ) );
        if( !item )
            return -1;
        val_key = item.get_item( 0 );
        PyObjectPtr val_val( item.get_item( 1 ) );
        return PyDict_Type.tp_as_mapping->mp_ass_subscript(
            m_dict.get(), val_key.get(), val_val.get() );
    }

protected:

    AtomDict* adict()
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

    PyObject* validate_key( PyObject* key )
    {
        PyObjectPtr val_key( newref(key) );
        if( key_validator() && atom() )
        {
            val_key = key_validator()->full_validate( atom(), Py_None, key );
            if( !val_key )
                return 0;
        }

        return val_key.release();
    }

    PyObject* validate_key_value_pair( PyObject* key , PyObject* value )
    {
        PyDictPtr item( PyDict_New() );
        PyObjectPtr val_key( newref(key) );
        PyObjectPtr val_val( newref(value) );
        PyTuplePtr pair( PyTuple_New( 2 ) );
        if( key_validator() && atom() )
        {
            val_key = key_validator()->full_validate( atom(), Py_None, key );
            if( !val_key )
                return 0;
        }
        if( value_validator() && atom() )
        {
            val_val = value_validator()->full_validate( atom(), Py_None, value );
            if( !val_val )
                return 0;
        }
        item.set_item( val_key, val_val );
        m_validated = item;
        pair.initialize( 0, val_key );
        pair.initialize( 1, val_val );
        return pair.release();
    }

    PyObject* validate_pairs( PyObject* value )
    {
        PyListPtr temppairs( PySequence_List( value ));
        if( !temppairs )
                return 0;
        PyDictPtr item( PyDict_New() );
        if( ( key_validator() || value_validator() )  && atom() )
        {

            CAtom* atm = atom();
            Py_ssize_t size = temppairs.size();
            if ( key_validator() && value_validator() )
            {
                Member* key_vd = key_validator();
                Member* val_vd = value_validator();
                for( Py_ssize_t i = 0; i < size; ++i )
                {
                    PyObject* pair = temppairs.borrow_item( i );
                    PyObject* key = PySequence_GetItem( pair, 0 );
                    PyObject* val_key = key_vd->full_validate( atm, Py_None, key );
                    if( !val_key )
                        return 0;
                    PyObject* val = PySequence_GetItem( pair, 1 );
                    PyObject* val_val = val_vd->full_validate( atm, Py_None, val);
                    if( !val_val )
                        return 0;
                    item.set_item(val_key, val_val);
                }
            }
            else if ( key_validator() )
            {
                Member* key_vd = key_validator();
                for( Py_ssize_t i = 0; i < size; ++i )
                {
                    PyObject* pair = temppairs.borrow_item( i );
                    PyObject* key = PySequence_GetItem( pair, 0 );
                    PyObject* val_key = key_vd->full_validate( atm, Py_None, key );
                    if( !val_key )
                        return 0;
                    PyObject* val = PySequence_GetItem( pair, 1 );
                    item.set_item(val_key, val);
                }
            }
            else
            {
                Member* val_vd = value_validator();
                for( Py_ssize_t i = 0; i < size; ++i )
                {
                    PyObject* pair = temppairs.borrow_item( i );
                    PyObject* key = PySequence_GetItem( pair, 0 );

                    PyObject* val = PySequence_GetItem( pair, 1 );
                    PyObject* val_val = val_vd->full_validate( atm, Py_None, val);
                    if( !val_val )
                        return 0;
                    item.set_item(key, val_val);
                }
            }
        }

        else
        {
            if( !PyDict_MergeFromSeq2( item.get(), temppairs.get(), 0) )
                return 0;
        }
        m_validated = item;
        PyListPtr pairs( PyDict_Items( item.get() ) );
        return pairs.release();
    }

    PyObject* validate_dict( PyObject* dict )
    {
        PyDictPtr item( newref( dict ) );
        if( ( key_validator() || value_validator() )  && atom() )
        {

            if( m_dict.get() != dict )
            {
                PyDictPtr dictptr( PyDict_New() );
                CAtom* atm = atom();
                PyObject* key;
                PyObject* value;
                Py_ssize_t pos = 0;
            
                if( key_validator() && value_validator() )
                {
                    Member* key_vd = key_validator();
                    Member* val_vd = value_validator();
                    while( PyDict_Next( dict, &pos, &key, &value ) )
                        {
                            PyObjectPtr keyptr( key_vd->full_validate( atm, Py_None, key ) );
                            if( !keyptr )
                                return 0;
                            PyObjectPtr valptr( val_vd->full_validate( atm, Py_None, value ) );
                            if( !valptr )
                                return 0;
                            if( !dictptr.set_item( keyptr, valptr ) )
                                return 0;
                        }
                }
                else if( key_validator() )
                {
                    Member* key_vd = key_validator();
                    while( PyDict_Next( dict, &pos, &key, &value ) )
                        {
                            PyObjectPtr keyptr( key_vd->full_validate( atm, Py_None, key ) );
                            if( !keyptr )
                                return 0;
                            PyObjectPtr valptr( newref( value ) );
                            if( !dictptr.set_item( keyptr, valptr ) )
                                return 0;
                        }
                }
                else if( value_validator() )
                {
                    Member* val_vd = value_validator();
                    while( PyDict_Next( dict, &pos, &key, &value ) )
                        {
                            PyObjectPtr keyptr( newref( key ) );
                            PyObjectPtr valptr( val_vd->full_validate( atm, Py_None, value ) );
                            if( !valptr )
                                return 0;
                            if( !dictptr.set_item( keyptr, valptr ) )
                                return 0;
                        }
                }
                item = dictptr;
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
    atomdict_cast( ptr.get() )->pointer = new CAtomPointer();
    return ptr.release();
}


static void
AtomDict_dealloc( AtomDict* self )
{
    delete self->pointer;
    self->pointer = 0;
    Py_CLEAR( self->key_validator );
    Py_CLEAR( self->value_validator );
    PyDict_Type.tp_dealloc( pyobject_cast( self ) );
}

static PyObject*
AtomDict_setdefault( AtomDict* self, PyObject* args )
{
    return AtomDictHandler( self ).setdefault( args );
}

static PyObject*
AtomDict_update( AtomDict* self, PyObject* args, PyObject *kwargs )
{
    return AtomDictHandler( self ).update( args, kwargs );
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


PyDoc_STRVAR(a_setdefault_doc,
"D.setdefault(k[,d]) -> D.get(k,d), also set D[k]=d if k not in D");

PyDoc_STRVAR(a_update_doc,
"D.update(E, **F) -> None. Update D from dict/iterable E and F.\n"
"If E has a .keys() method, does: for k in E: D[k] = E[k]\n\
If E lacks .keys() method, does: for (k, v) in E: D[k] = v\n\
In either case, this is followed by: for k in F: D[k] = F[k]");


static PyMethodDef
AtomDict_methods[] = {
    { "setdefault", ( PyCFunction )AtomDict_setdefault, METH_VARARGS, a_setdefault_doc },
    { "update", ( PyCFunction )AtomDict_update, METH_VARARGS | METH_KEYWORDS, a_update_doc },
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
    if( !DictMethods::init_methods() )
        return -1;
    return 0;
}