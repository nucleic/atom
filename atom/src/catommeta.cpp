/*-----------------------------------------------------------------------------
 | Copyright (c) 2025, Nucleic Development Team.                           *
 |
 | Distributed under the terms of the Modified BSD License.
 |
 | The full license is in the file LICENSE, distributed with this software.
 |----------------------------------------------------------------------------*/
#include "catommeta.h"
#include "member.h"
#include "packagenaming.h"

namespace atom
{

namespace
{

static PyObject* atom_members_str;

static PyObject* invalid_members_error()
{
    return cppy::system_error("CAtomMeta members are not initialized. Subclasses that use __init_subclass__ must call super().__init_subclass__()");
}

void
CAtomMeta_clear( CAtomMeta* self )
{
    Py_CLEAR( self->atom_members );
}

int
CAtomMeta_traverse( CAtomMeta* self, visitproc visit, void* arg )
{
    Py_VISIT( self->atom_members );
    return 0;
}

void
CAtomMeta_dealloc( CAtomMeta* self )
{
    PyObject_GC_UnTrack( self );
    CAtomMeta_clear( self );
    Py_TYPE(self)->tp_free( pyobject_cast( self ) );
}

PyObject*
CAtomMeta_get_atom_members( CAtomMeta* self, void* context )
{
    if ( !self->atom_members )
        return invalid_members_error();
    return cppy::incref( self->atom_members );
}

// Validates the members argument and returns the slot count required.
// If an error occurs it returns -1 and sets an error
int
CAtomMeta_validate_members( CAtomMeta* self, PyObject* members )
{
    if ( !PyDict_CheckExact( members ) )
    {
        cppy::type_error("CAtomMeta __atom_members__ must be a dict");
        return -1;
    }

    PyObject *key, *value;
    uint32_t count = 0;
    Py_ssize_t pos = 0;
    utils::CriticalSection lock( members );
    while ( PyDict_Next( members, &pos, &key, &value ) )
    {
        if ( !PyUnicode_CheckExact( key ) )
        {
            cppy::type_error("CAtomMeta __atom_members__ key must be a str");
            return -1;
        }
        if ( !Member::TypeCheck( value ) )
        {
            cppy::type_error("CAtomMeta __atom_members__ value must be a Member");
            return -1;
        }

        // Members that don't require storage can set the index over the limit
        Member* member = reinterpret_cast<Member*>( value );
        if ( member->index < MAX_MEMBER_COUNT )
            count += 1;
    }
    if (count > MAX_MEMBER_COUNT)
    {
        cppy::type_error("CAtomMeta __atom_members__ has too many members");
        return -1;
    }
    return count;
}

int
CAtomMeta_set_atom_members( CAtomMeta* self, PyObject* members, void* context )
{
    int count = CAtomMeta_validate_members( self, members );
    if ( count < 0 )
        return -1;
    self->atom_members = cppy::incref( members );
    self->slot_count = count;
    return 0;
}

PyObject*
CAtomMeta_init_subclass( CAtomMeta* self )
{
    // Since type_new does a copy of the attrs and does not use setattr
    // we have to manually move __atom_members__ from the internal dict
    // to the our managed self->atom_members pointer
    if ( !self->atom_members )
    {
        if ( PyObject** dict = _PyObject_GetDictPtr( pyobject_cast(self) ) )
        {
            if ( PyObject* members = PyDict_GetItem( *dict, atom_members_str ) )
            {
                if ( CAtomMeta_set_atom_members( self , members, 0 ) < 0 )
                    return 0;
                if ( PyDict_DelItem( *dict, atom_members_str ) < 0)
                    return 0;  // LCOV_EXCL_LINE
            }
        }

    }
    // If it was not set return an error
    if ( !self->atom_members )
        return invalid_members_error();
    Py_RETURN_NONE;
}

// Updates the slot
bool
CAtomMeta_update_slot_count( CAtomMeta* self )
{
    if ( !self->atom_members )
        return invalid_members_error();
    int count = CAtomMeta_validate_members( self, self->atom_members );
    if ( count < 0 )
        return false;
    self->slot_count = count;
    return true;
}

PyObject*
CAtomMeta_get_member( CAtomMeta* self, PyObject* name )
{
    if ( !PyUnicode_Check(name) )
        return cppy::type_error("get_member name must be a str");
    if ( !self->atom_members )
        return invalid_members_error();
    PyObject* member = PyDict_GetItem( self->atom_members, name );
    if ( !member )
        Py_RETURN_NONE;
    return cppy::incref(member);
}

PyObject*
CAtomMeta_set_member( CAtomMeta* self, PyObject*const *args,  Py_ssize_t n )
{
    if ( n != 2 || !PyUnicode_Check(args[0]) || !Member::TypeCheck(args[1]) )
        return cppy::type_error("set_member requires 2 arguments name, member");
    if ( !self->atom_members )
        return invalid_members_error();
    if ( PyDict_SetItem( self->atom_members, args[0], args[1] ) < 0 )
        return 0;
    if ( !CAtomMeta_update_slot_count( self ) )
        return 0;
    Py_RETURN_NONE;
}

PyObject*
CAtomMeta_del_member( CAtomMeta* self, PyObject* name )
{
    if ( !PyUnicode_Check(name) )
        return cppy::type_error("del_member name must be a str");
    if ( !self->atom_members )
        return invalid_members_error();
    if ( PyDict_DelItem( self->atom_members, name ) < 0 )
        return 0;
    if ( !CAtomMeta_update_slot_count( self ) )
        return 0;
    Py_RETURN_NONE;
}

static PyGetSetDef
CAtomMeta_getset[] = {
    { "__atom_members__", ( getter )CAtomMeta_get_atom_members, ( setter )CAtomMeta_set_atom_members,
        "Get and set the atom members." },
    { 0 } // sentinel
};

static PyMethodDef
CAtomMeta_methods[] = {

    { "get_member", ( PyCFunction )CAtomMeta_get_member, METH_O,
        "Get the member with the given name." },
    { "__add_member", ( PyCFunction )CAtomMeta_set_member, METH_FASTCALL,
        "Set the member with the given name." },
    { "__del_member", ( PyCFunction )CAtomMeta_del_member, METH_O,
            "Delete the member with the given name." },
    { "members", ( PyCFunction )CAtomMeta_get_atom_members, METH_NOARGS,
        "Get all the members." },
    { 0 } // sentinel
};

static PyType_Slot CAtomMeta_Type_slots[] = {
    { Py_tp_dealloc, void_cast( CAtomMeta_dealloc ) },
    { Py_tp_traverse, void_cast( CAtomMeta_traverse ) },
    { Py_tp_clear, void_cast( CAtomMeta_clear ) },
    { Py_tp_methods, void_cast( CAtomMeta_methods ) },
    { Py_tp_getset, void_cast( CAtomMeta_getset ) },
    { Py_tp_free, void_cast( PyObject_GC_Del ) },
    { 0, 0 },
};

} // namespace

// Initialize static variables (otherwise the compiler eliminates them)
PyTypeObject* CAtomMeta::TypeObject = NULL;

PyType_Spec CAtomMeta::TypeObject_Spec = {
    .name = PACKAGE_TYPENAME( "CAtomMeta" ),
    .basicsize = sizeof( CAtomMeta ),
    .itemsize = 0,
    .flags = Py_TPFLAGS_DEFAULT |Py_TPFLAGS_BASETYPE |Py_TPFLAGS_HAVE_GC,
    .slots = CAtomMeta_Type_slots
};

bool CAtomMeta::Ready()
{
    atom_members_str = PyUnicode_InternFromString( "__atom_members__" );
    if ( !atom_members_str )
        return 0;
    TypeObject = pytype_cast( PyType_FromSpecWithBases( &TypeObject_Spec, pyobject_cast( &PyType_Type ) ) );
    return TypeObject != 0;
}

PyObject*
CAtomMeta::members( )
{
    return CAtomMeta_get_atom_members( this, 0 );
}

PyObject*
CAtomMeta::get_member( PyObject* name )
{
    return CAtomMeta_get_member( this, name );
}

PyObject*
CAtomMeta::init_subclass()
{
    return CAtomMeta_init_subclass( this );
}

} // namespace atom
