/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2017, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once
#include <Python.h>
#include <bytesobject.h>
#include <structmember.h>
#include <string>


#ifndef Py_RETURN_NOTIMPLEMENTED
#define Py_RETURN_NOTIMPLEMENTED \
    return Py_INCREF(Py_NotImplemented), Py_NotImplemented
#endif

#ifndef cmpfunc
#define cmpfunc int
#endif


#define pyobject_cast( o ) ( reinterpret_cast<PyObject*>( o ) )
#define pytype_cast( o ) ( reinterpret_cast<PyTypeObject*>( o ) )

namespace PythonHelpers
{


/*-----------------------------------------------------------------------------
| Exception Handling
|----------------------------------------------------------------------------*/
inline PyObject*
py_bad_internal_call( const char* message )
{
    PyErr_SetString( PyExc_SystemError, message );
    return 0;
}


inline PyObject*
py_type_fail( const char* message )
{
    PyErr_SetString( PyExc_TypeError, message );
    return 0;
}


inline PyObject*
py_expected_type_fail( PyObject* pyobj, const char* expected_type )
{
    PyErr_Format(
        PyExc_TypeError,
        "Expected object of type `%s`. Got object of type `%s` instead.",
        expected_type, pyobj->ob_type->tp_name
    );
    return 0;
}


inline PyObject*
py_value_fail( const char* message )
{
    PyErr_SetString( PyExc_ValueError, message );
    return 0;
}


inline PyObject*
py_runtime_fail( const char* message )
{
    PyErr_SetString( PyExc_RuntimeError, message );
    return 0;
}


inline PyObject*
py_attr_fail( const char* message )
{
    PyErr_SetString( PyExc_AttributeError, message );
    return 0;
}


inline PyObject*
py_no_attr_fail( PyObject* pyobj, const char* attr )
{
    PyErr_Format(
        PyExc_AttributeError,
        "'%s' object has no attribute '%s'",
        pyobj->ob_type->tp_name, attr
    );
    return 0;
}


/*-----------------------------------------------------------------------------
| Utilities
|----------------------------------------------------------------------------*/
inline PyObject*
newref( PyObject* pyobj )
{
    Py_INCREF( pyobj );
    return pyobj;
}


inline PyObject*
xnewref( PyObject* pyobj )
{
    Py_XINCREF( pyobj );
    return pyobj;
}


inline PyObject*
py_bool( bool val )
{
    return newref( val ? Py_True : Py_False );
}


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

    
/**
 * A fallback 3way comparison function for when PyObject_RichCompareBool
 * fails to compare "unorderable types" on Python 3.
 *
 * This is based on Python 2's `default_3way_compare`.
 *
 * This function will not change the Python exception state.
 */
inline int
fallback_3way_compare( PyObject* first, PyObject* second ) {
    // Compare pointer values if the types are the same.
    if( first->ob_type == second->ob_type ) {
        Py_uintptr_t fp = reinterpret_cast<Py_uintptr_t>( first );
        Py_uintptr_t sp = reinterpret_cast<Py_uintptr_t>( second );
        return (fp < sp) ? -1 : (fp > sp) ? 1 : 0;
    }

    // None is smaller than anything.
    if( first == Py_None )
        return -1;
    if( second == Py_None )
        return 1;

    // Compare based on type names, numbers are smaller.
    const char* fn = PyNumber_Check( first ) ? "" : first->ob_type->tp_name;
    const char* sn = PyNumber_Check( second ) ? "" : second->ob_type->tp_name;
    int c = strcmp( fn, sn );
    if( c < 0 )
        return -1;
    if( c > 0 )
        return 1;

    // Finally, fall back to comparing type pointers.
    Py_uintptr_t ftp = reinterpret_cast<Py_uintptr_t>( first->ob_type );
    Py_uintptr_t stp = reinterpret_cast<Py_uintptr_t>( second->ob_type );
    return ftp < stp ? -1 : 1;
}


/*-----------------------------------------------------------------------------
| Object Ptr
|----------------------------------------------------------------------------*/
class PyObjectPtr {

public:

    PyObjectPtr() : m_pyobj( 0 ) {}

    PyObjectPtr( const PyObjectPtr& objptr ) :
        m_pyobj( PythonHelpers::xnewref( objptr.m_pyobj ) ) {}

    PyObjectPtr( PyObject* pyobj ) : m_pyobj( pyobj ) {}

    ~PyObjectPtr()
    {
        xdecref_release();
    }

    PyObject* get() const
    {
        return m_pyobj;
    }

    void set( PyObject* pyobj )
    {
        PyObject* old = m_pyobj;
        m_pyobj = pyobj;
        Py_XDECREF( old );
    }

    PyObject* release()
    {
        PyObject* pyobj = m_pyobj;
        m_pyobj = 0;
        return pyobj;
    }

    PyObject* decref_release()
    {
        PyObject* pyobj = m_pyobj;
        m_pyobj = 0;
        Py_DECREF( pyobj );
        return pyobj;
    }

    PyObject* xdecref_release()
    {
        PyObject* pyobj = m_pyobj;
        m_pyobj = 0;
        Py_XDECREF( pyobj );
        return pyobj;
    }

    PyObject* incref_release()
    {
        PyObject* pyobj = m_pyobj;
        m_pyobj = 0;
        Py_INCREF( pyobj );
        return pyobj;
    }

    PyObject* xincref_release()
    {
        PyObject* pyobj = m_pyobj;
        m_pyobj = 0;
        Py_XINCREF( pyobj );
        return pyobj;
    }

    void incref() const
    {
        Py_INCREF( m_pyobj );
    }

    void decref() const
    {
        Py_DECREF( m_pyobj );
    }

    void xincref() const
    {
        Py_XINCREF( m_pyobj );
    }

    void xdecref() const
    {
        Py_XDECREF( m_pyobj );
    }

    PyObject* newref() const
    {
        Py_INCREF( m_pyobj );
        return m_pyobj;
    }

    PyObject* xnewref() const
    {
        Py_XINCREF( m_pyobj );
        return m_pyobj;
    }

    size_t refcount() const
    {
        if( m_pyobj )
            return m_pyobj->ob_refcnt;
        return 0;
    }

    bool is_true( bool clear_err=true ) const
    {
        int truth = PyObject_IsTrue( m_pyobj );
        if( truth == 1 )
            return true;
        if( truth == 0 )
            return false;
        if( clear_err )
            PyErr_Clear();
        return false;
    }

    bool is_None() const
    {
        return m_pyobj == Py_None;
    }

    bool is_True() const
    {
        return m_pyobj == Py_True;
    }

    bool is_False() const
    {
        return m_pyobj == Py_False;
    }

    bool load_dict( PyObjectPtr& out, bool forcecreate=false )
    {
        PyObject** dict = _PyObject_GetDictPtr( m_pyobj );
        if( !dict )
            return false;
        if( forcecreate && !*dict )
            *dict = PyDict_New();
        out = PythonHelpers::xnewref( *dict );
        return true;
    }

    bool richcompare( PyObject* other, int opid, bool clear_err=true )
    {
        // Start with Python's rich compare.
        int r = PyObject_RichCompareBool( m_pyobj, other, opid );

        // Handle a successful comparison.
        if( r == 1 )
            return true;
        if( r == 0 )
            return false;

        // Clear the error if requested.
        if( clear_err && PyErr_Occurred() )
            PyErr_Clear();

        // Fallback to the Python 2 default 3 way compare.
        int c = fallback_3way_compare( m_pyobj, other );

        // Convert the 3way comparison result based on the `opid`.
        switch (opid) {
        case Py_EQ: return c == 0;
        case Py_NE: return c != 0;
        case Py_LE: return c <= 0;
        case Py_GE: return c >= 0;
        case Py_LT: return c < 0;
        case Py_GT: return c > 0;
        }

        // Return `false` if the `opid` is not handled.
        return false;
    }

    bool richcompare( PyObjectPtr& other, int opid, bool clear_err=true )
    {
        return richcompare( other.m_pyobj, opid, clear_err );
    }

    bool hasattr( PyObject* attr )
    {
        return PyObject_HasAttr( m_pyobj, attr ) == 1;
    }

    bool hasattr( PyObjectPtr& attr )
    {
        return PyObject_HasAttr( m_pyobj, attr.get() ) == 1;
    }

    bool hasattr( const char* attr )
    {
        return PyObject_HasAttrString( m_pyobj, attr ) == 1;
    }

    bool hasattr( std::string& attr )
    {
        return hasattr( attr.c_str() );
    }

    PyObjectPtr getattr( PyObject* attr )
    {
        return PyObjectPtr( PyObject_GetAttr( m_pyobj, attr ) );
    }

    PyObjectPtr getattr( PyObjectPtr& attr )
    {
        return PyObjectPtr( PyObject_GetAttr( m_pyobj, attr.get() ) );
    }

    PyObjectPtr getattr( const char* attr )
    {
        return PyObjectPtr( PyObject_GetAttrString( m_pyobj, attr ) );
    }

    PyObjectPtr getattr( std::string& attr )
    {
        return getattr( attr.c_str() );
    }

    bool setattr( PyObject* attr, PyObject* value )
    {
        return PyObject_SetAttr( m_pyobj, attr, value ) == 0;
    }

    bool setattr( PyObjectPtr& attr, PyObjectPtr& value )
    {
        return PyObject_SetAttr( m_pyobj, attr.get(), value.get() ) == 0;
    }

    PyObjectPtr operator()( PyObjectPtr& args ) const
    {
        return PyObjectPtr( PyObject_Call( m_pyobj, args.get(), 0 ) );
    }

    PyObjectPtr operator()( PyObjectPtr& args, PyObjectPtr& kwargs ) const
    {
        return PyObjectPtr( PyObject_Call( m_pyobj, args.get(), kwargs.get() ) );
    }

    operator void*() const
    {
        return static_cast<void*>( m_pyobj );
    }

    PyObjectPtr& operator=( const PyObjectPtr& rhs )
    {
        PyObject* old = m_pyobj;
        m_pyobj = rhs.m_pyobj;
        Py_XINCREF( m_pyobj );
        Py_XDECREF( old );
        return *this;
    }

    PyObjectPtr& operator=( PyObject* rhs )
    {
        PyObject* old = m_pyobj;
        m_pyobj = rhs;
        Py_XDECREF( old );
        return *this;
    }

protected:

    PyObject* m_pyobj;

};


inline bool
operator!=( const PyObjectPtr& lhs, const PyObjectPtr& rhs )
{
    return lhs.get() != rhs.get();
}


inline bool
operator!=( const PyObject* lhs, const PyObjectPtr& rhs )
{
    return lhs != rhs.get();
}


inline bool
operator!=( const PyObjectPtr& lhs, const PyObject* rhs )
{
    return lhs.get() != rhs;
}


inline bool
operator==( const PyObjectPtr& lhs, const PyObjectPtr& rhs )
{
    return lhs.get() == rhs.get();
}


inline bool
operator==( const PyObject* lhs, const PyObjectPtr& rhs )
{
    return lhs == rhs.get();
}


inline bool
operator==( const PyObjectPtr& lhs, const PyObject* rhs )
{
    return lhs.get() == rhs;
}


/*-----------------------------------------------------------------------------
| Tuple Ptr
|----------------------------------------------------------------------------*/
class PyTuplePtr : public PyObjectPtr {

public:

    PyTuplePtr() : PyObjectPtr() {}

    PyTuplePtr( const PyObjectPtr& objptr ) : PyObjectPtr( objptr ) {}

    PyTuplePtr( PyObject* pytuple ) : PyObjectPtr( pytuple ) {}

    bool check()
    {
        return PyTuple_Check( m_pyobj );
    }

    bool check_exact()
    {
        return PyTuple_CheckExact( m_pyobj );
    }

    Py_ssize_t size() const
    {
        return PyTuple_GET_SIZE( m_pyobj );
    }

    PyObjectPtr get_item( Py_ssize_t index ) const
    {
        return PyObjectPtr( PythonHelpers::newref( PyTuple_GET_ITEM( m_pyobj, index ) ) );
    }

    void set_item( Py_ssize_t index, PyObject* pyobj )
    {
        PyObject* old_item = PyTuple_GET_ITEM( m_pyobj, index );
        PyTuple_SET_ITEM( m_pyobj, index, pyobj );
        Py_XDECREF( old_item );
    }

    void set_item( Py_ssize_t index, PyObjectPtr& item )
    {
        PyObject* old_item = PyTuple_GET_ITEM( m_pyobj, index );
        PyTuple_SET_ITEM( m_pyobj, index, item.get() );
        Py_XINCREF( item.get() );
        Py_XDECREF( old_item );
    }

    // pyobj must not be null, only use to fill a new empty tuple
    void initialize( Py_ssize_t index, PyObject* pyobj )
    {
        PyTuple_SET_ITEM( m_pyobj, index, pyobj );
    }

    // ptr must not be empty, only use to fill a new empty tuple
    void initialize( Py_ssize_t index, PyObjectPtr& item )
    {
        PyTuple_SET_ITEM( m_pyobj, index, item.get() );
        Py_INCREF( item.get() );
    }

};


/*-----------------------------------------------------------------------------
| List Ptr
|----------------------------------------------------------------------------*/
class PyListPtr : public PyObjectPtr {

public:

    PyListPtr() : PyObjectPtr() {}

    PyListPtr( const PyObjectPtr& objptr ) : PyObjectPtr( objptr ) {}

    PyListPtr( PyObject* pylist ) : PyObjectPtr( pylist ) {}

    bool check() const
    {
        return PyList_Check( m_pyobj );
    }

    bool check_exact() const
    {
        return PyList_CheckExact( m_pyobj );
    }

    Py_ssize_t size() const
    {
        return PyList_GET_SIZE( m_pyobj );
    }

    PyObject* borrow_item( Py_ssize_t index ) const
    {
        return PyList_GET_ITEM( m_pyobj, index );
    }

    PyObjectPtr get_item( Py_ssize_t index ) const
    {
        return PyObjectPtr( PythonHelpers::newref( PyList_GET_ITEM( m_pyobj, index ) ) );
    }

    void set_item( Py_ssize_t index, PyObject* pyobj ) const
    {
        PyObject* old_item = PyList_GET_ITEM( m_pyobj, index );
        PyList_SET_ITEM( m_pyobj, index, pyobj );
        Py_XDECREF( old_item );
    }

    void set_item( Py_ssize_t index, PyObjectPtr& item ) const
    {
        PyObject* old_item = PyList_GET_ITEM( m_pyobj, index );
        PyList_SET_ITEM( m_pyobj, index, item.get() );
        Py_XINCREF( item.get() );
        Py_XDECREF( old_item );
    }

    bool del_item( Py_ssize_t index ) const
    {
        if( PySequence_DelItem( m_pyobj, index ) == -1 )
            return false;
        return true;
    }

    bool append( PyObjectPtr& pyobj ) const
    {
        if( PyList_Append( m_pyobj, pyobj.get() ) == 0 )
            return true;
        return false;
    }

    Py_ssize_t index( PyObjectPtr& item ) const
    {
        Py_ssize_t maxidx = size();
        for( Py_ssize_t idx = 0; idx < maxidx; idx++ )
        {
            PyObjectPtr other( get_item( idx ) );
            if( item.richcompare( other, Py_EQ ) )
                return idx;
        }
        return -1;
    }

};


/*-----------------------------------------------------------------------------
| Dict Ptr
|----------------------------------------------------------------------------*/
class PyDictPtr : public PyObjectPtr {

public:

    PyDictPtr() : PyObjectPtr() {}

    PyDictPtr( const PyObjectPtr& objptr ) : PyObjectPtr( objptr ) {}

    PyDictPtr( PyObject* pydict ) : PyObjectPtr( pydict ) {}

    bool check()
    {
        return PyDict_Check( m_pyobj );
    }

    bool check_exact()
    {
        return PyDict_CheckExact( m_pyobj );
    }

    Py_ssize_t size() const
    {
        return PyDict_Size( m_pyobj );
    }

    PyObjectPtr get_item( PyObject* key ) const
    {
        return PyObjectPtr( PythonHelpers::xnewref( PyDict_GetItem( m_pyobj, key ) ) ) ;
    }

    PyObjectPtr get_item( PyObjectPtr& key ) const
    {
        return PyObjectPtr( PythonHelpers::xnewref( PyDict_GetItem( m_pyobj, key.get() ) ) );
    }

    PyObjectPtr get_item( const char* key ) const
    {
        return PyObjectPtr( PythonHelpers::xnewref( PyDict_GetItemString( m_pyobj, key ) ) );
    }

    PyObjectPtr get_item( std::string& key ) const
    {
        return get_item( key.c_str() );
    }

    bool set_item( PyObject* key, PyObject* value ) const
    {
        if( PyDict_SetItem( m_pyobj, key, value ) == 0 )
            return true;
        return false;
    }

    bool set_item( PyObject* key, PyObjectPtr& value ) const
    {
        if( PyDict_SetItem( m_pyobj, key, value.get() ) == 0 )
            return true;
        return false;
    }

    bool set_item( PyObjectPtr& key, PyObject* value ) const
    {
        if( PyDict_SetItem( m_pyobj, key.get(), value ) == 0 )
            return true;
        return false;
    }

    bool set_item( PyObjectPtr& key, PyObjectPtr& value ) const
    {
        if( PyDict_SetItem( m_pyobj, key.get(), value.get() ) == 0 )
            return true;
        return false;
    }

    bool set_item( const char* key, PyObjectPtr& value ) const
    {
        if( PyDict_SetItemString( m_pyobj, key, value.get() ) == 0 )
            return true;
        return false;
    }

    bool set_item( const char* key, PyObject* value ) const
    {
        if( PyDict_SetItemString( m_pyobj, key, value ) == 0 )
            return true;
        return false;
    }

    bool set_item( std::string& key, PyObjectPtr& value ) const
    {
        return set_item( key.c_str(), value );
    }

    bool del_item( PyObjectPtr& key ) const
    {
        if( PyDict_DelItem( m_pyobj, key.get() ) == 0 )
            return true;
        return false;
    }

    bool del_item( const char* key ) const
    {
        if( PyDict_DelItemString( m_pyobj, key ) == 0 )
            return true;
        return false;
    }

    bool del_item( std::string& key ) const
    {
        return del_item( key.c_str() );
    }

};


/* TODO - is there a better way than this?? */
#ifndef PyMethod_GET_CLASS
#define PyMethod_GET_CLASS(m) (PyObject*)Py_TYPE(PyMethod_GET_SELF(m))
#endif

/*-----------------------------------------------------------------------------
| Method Ptr
|----------------------------------------------------------------------------*/
class PyMethodPtr : public PyObjectPtr {

public:

    PyMethodPtr() : PyObjectPtr() {}

    PyMethodPtr( const PyObjectPtr& objptr ) : PyObjectPtr( objptr ) {}

    PyMethodPtr( PyObject* pymethod ) : PyObjectPtr( pymethod ) {}

    bool check()
    {
        return PyMethod_Check( m_pyobj );
    }

    PyObjectPtr get_self() const
    {
        return PyObjectPtr( PythonHelpers::xnewref( PyMethod_GET_SELF( m_pyobj ) ) );
    }

    PyObjectPtr get_function() const
    {
        return PyObjectPtr( PythonHelpers::xnewref( PyMethod_GET_FUNCTION( m_pyobj ) ) );
    }

    PyObjectPtr get_class() const
    {
        return PyObjectPtr( PythonHelpers::xnewref( PyMethod_GET_CLASS( m_pyobj ) ) );
    }

};


/*-----------------------------------------------------------------------------
| Weakref Ptr
|----------------------------------------------------------------------------*/
class PyWeakrefPtr : public PyObjectPtr {

public:

    PyWeakrefPtr() : PyObjectPtr() {}

    PyWeakrefPtr( const PyObjectPtr& objptr ) : PyObjectPtr( objptr ) {}

    PyWeakrefPtr( PyObject* pyweakref ) : PyObjectPtr( pyweakref ) {}

    bool check()
    {
        return PyWeakref_CheckRef( m_pyobj );
    }

    bool check_exact()
    {
        return PyWeakref_CheckRefExact( m_pyobj );
    }

    PyObjectPtr get_object() const
    {
        return PyObjectPtr( PythonHelpers::newref( PyWeakref_GET_OBJECT( m_pyobj ) ) );
    }

};

} // namespace PythonHelpers
