/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2025, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file LICENSE, distributed with this software.
|----------------------------------------------------------------------------*/
#include <map>
#include <iostream>
#include <sstream>
#include <cppy/cppy.h>
#include "atomref.h"
#include "catompointer.h"
#include "globalstatic.h"
#include "packagenaming.h"

#ifdef __clang__
#pragma clang diagnostic ignored "-Wdeprecated-writable-strings"
#endif

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif


#define atomref_cast( o ) ( reinterpret_cast<atom::AtomRef*>( o ) )


namespace atom
{


namespace SharedAtomRef
{

typedef std::map<CAtom*, cppy::ptr> RefMap;
GLOBAL_STATIC( RefMap, ref_map )


PyObject*
get( CAtom* atom )
{
    if( atom->has_atomref() )
    {
        return cppy::incref( ( *ref_map() )[ atom ].get() );
    }
    PyObject* pyref = PyType_GenericAlloc( AtomRef::TypeObject, 0 );
    if( !pyref )
    {
        return 0;
    }
    // placement new since Python malloc'd and zero'd the struct
    new( &atomref_cast( pyref )->pointer ) CAtomPointer( atom );
    atomref_cast( pyref )->vectorcall = AtomRef::call;
    ( *ref_map() )[ atom ] = cppy::incref( pyref );
    atom->set_has_atomref( true );
    return pyref;
}


void
clear( CAtom* atom )
{
    ref_map()->erase( atom );
    atom->set_has_atomref( false );
}

}  // namespace SharedAtomRef

namespace
{


PyObject*
AtomRef_new( PyTypeObject* type, PyObject* args, PyObject* kwargs )
{
    static char *kwlist[] = { "atom", 0 };
    PyObject* atom;
    if( !PyArg_ParseTupleAndKeywords( args, kwargs, "O:__new__", kwlist, &atom ) )
    {
        return 0;
    }
    if( !CAtom::TypeCheck( atom ) )
    {
        return cppy::type_error( atom, "CAtom" );
    }
    return SharedAtomRef::get( catom_cast( atom ) );
}


PyObject*
AtomRef_vnew( PyObject* type, PyObject*const *args, size_t nargsf, PyObject* kwnames )
{
    if ( kwnames )
        return cppy::type_error("atomref takes no kwargs");
    if ( PyVectorcall_NARGS(nargsf) != 1 )
        return cppy::type_error("atomref takes exactly 1 argument");
    PyObject* atom = args[0];
    if ( !CAtom::TypeCheck( atom ) )
        return cppy::type_error( atom, "CAtom" );
    return SharedAtomRef::get( catom_cast( atom ) );
}

void
AtomRef_dealloc( AtomRef* self )
{
    // manual destructor since Python malloc'd and zero'd the struct
    PyTypeObject* tp = Py_TYPE( self );
    self->pointer.~CAtomPointer();
    tp->tp_free( pyobject_cast( self ) );
    Py_DECREF( tp );
}


PyObject*
AtomRef_call( AtomRef* self, PyObject*const *args, size_t nargsf, PyObject* kwnames )
{
    if ( kwnames || PyVectorcall_NARGS(nargsf) )
        return cppy::type_error("atomref.__call__() takes no args or kwargs");
    PyObject* obj = pyobject_cast( self->pointer.data() );
    return cppy::incref( obj ? obj : Py_None );
}


PyObject*
AtomRef_repr( AtomRef* self )
{
    std::ostringstream ostr;
    ostr << "AtomRef(atom=";
    if( self->pointer.is_null() )
    {
        ostr << "None";
    }
    else
    {
        PyObject* obj = pyobject_cast( self->pointer.data() );
        cppy::ptr repr( PyObject_Repr( obj ) );
        if( !repr )
            return 0;
        const char* s = PyUnicode_AsUTF8( repr.get() );
        if ( !s )
            return 0;
        ostr << s;
    }
    ostr << ")";
    return PyUnicode_FromString( ostr.str().c_str() );
}


PyObject*
AtomRef_sizeof( AtomRef* self )
{
    Py_ssize_t size = Py_TYPE(self)->tp_basicsize;
    size += sizeof( CAtomPointer );
    return PyLong_FromSsize_t( size );
}


int
AtomRef__bool__( AtomRef* self )
{
    return self->pointer.is_null() ? 0 : 1;
}


static PyMethodDef
AtomRef_methods[] = {
    { "__sizeof__", ( PyCFunction )AtomRef_sizeof, METH_NOARGS,
      "__sizeof__() -> size of object in memory, in bytes" },
    { 0 } // sentinel
};


static PyType_Slot AtomRef_Type_slots[] = {
    { Py_tp_dealloc, void_cast( AtomRef_dealloc ) },              /* tp_dealloc */
    { Py_tp_repr, void_cast( AtomRef_repr ) },                    /* tp_repr */
    { Py_tp_methods, void_cast( AtomRef_methods ) },              /* tp_methods */
    { Py_tp_new, void_cast( AtomRef_new ) },                      /* tp_new */
#if defined(Py_tp_vectorcall)
    { Py_tp_vectorcall, void_cast( AtomRef_vnew ) },              /* tp_vectorcall */
#endif
    { Py_tp_call, void_cast( PyVectorcall_Call ) },               /* tp_call */
    { Py_tp_alloc, void_cast( PyType_GenericAlloc ) },            /* tp_alloc */
    { Py_nb_bool, void_cast( AtomRef__bool__ ) },                 /* nb_bool */
    { 0, 0 },
};

}  // namespace


PyTypeObject* AtomRef::TypeObject = NULL;


PyType_Spec AtomRef::TypeObject_Spec = {
	PACKAGE_TYPENAME( "atomref" ),             /* tp_name */
	sizeof( AtomRef ),                          /* tp_basicsize */
	0,                                          /* tp_itemsize */
	Py_TPFLAGS_DEFAULT
	| Py_TPFLAGS_HAVE_VECTORCALL,               /* tp_flags */
    AtomRef_Type_slots                          /* slots */
};

PyObject* AtomRef::call( PyObject* self, PyObject*const *args, size_t nargsf, PyObject* kwnames )
{
    return AtomRef_call( atomref_cast(self), args, nargsf, kwnames);
}

bool AtomRef::Ready()
{
    // The reference will be handled by the module to which we will add the type
	TypeObject = pytype_cast( PyType_FromSpec( &TypeObject_Spec ) );
    if( !TypeObject )
    {
        return false;  // LCOV_EXCL_LINE (failed type init)
    }
    TypeObject->tp_vectorcall_offset = offsetof(AtomRef, vectorcall);
    return true;
}


}
