/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2025, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file LICENSE, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once

#include <vector>
#include <cppy/cppy.h>
#include "inttypes.h"
#include "behaviors.h"
#include "catom.h"
#include "modifyguard.h"
#include "observer.h"

#ifndef UINT64_C
#define UINT64_C( c ) ( c ## ULL )
#endif

#define member_cast( o ) ( reinterpret_cast<Member*>( o ) )


namespace atom
{

PACK(struct MemberModes
{
    GetAttr::Mode getattr: 4;
    PostGetAttr::Mode post_getattr: 3;
    SetAttr::Mode setattr: 4;
    PostSetAttr::Mode post_setattr: 3;
    DefaultValue::Mode default_value: 4;
    Validate::Mode validate: 5;
    PostValidate::Mode post_validate: 3;
    DelAttr::Mode delattr: 3;
    GetState::Mode getstate: 3;
});

struct Member
{
    PyObject_HEAD
    PyObject* name;
    PyObject* metadata;
    PyObject* getattr_context;
    PyObject* setattr_context;
    PyObject* delattr_context;
    PyObject* validate_context;
    PyObject* post_getattr_context;
    PyObject* post_setattr_context;
    PyObject* default_value_context;
    PyObject* post_validate_context;
    PyObject* getstate_context;
    ModifyGuard<Member>* modify_guard;
    std::vector<Observer>* static_observers;
    MemberModes modes;
    uint32_t index;

    static PyType_Spec TypeObject_Spec;

    static PyTypeObject* TypeObject;

	static bool Ready();

    // ModifyGuard template interface
    ModifyGuard<Member>* get_modify_guard() { return modify_guard; }
    void set_modify_guard( ModifyGuard<Member>* guard ) { modify_guard = guard; }

    GetAttr::Mode get_getattr_mode()
    {
        return modes.getattr;
    }

    void set_getattr_mode( GetAttr::Mode mode )
    {
        modes.getattr = mode;
    }

    SetAttr::Mode get_setattr_mode()
    {
        return modes.setattr;
    }

    void set_setattr_mode( SetAttr::Mode mode )
    {
        modes.setattr = mode;
    }

    PostGetAttr::Mode get_post_getattr_mode()
    {
        return modes.post_getattr;
    }

    void set_post_getattr_mode( PostGetAttr::Mode mode )
    {
        modes.post_getattr = mode;
    }

    PostSetAttr::Mode get_post_setattr_mode()
    {
        return modes.post_setattr;
    }

    void set_post_setattr_mode( PostSetAttr::Mode mode )
    {
        modes.post_setattr = mode;
    }

    DefaultValue::Mode get_default_value_mode()
    {
        return modes.default_value;
    }

    void set_default_value_mode( DefaultValue::Mode mode )
    {
        modes.default_value = mode;
    }

    Validate::Mode get_validate_mode()
    {
        return modes.validate;
    }

    void set_validate_mode( Validate::Mode mode )
    {
        modes.validate = mode;
    }

    PostValidate::Mode get_post_validate_mode()
    {
        return modes.post_validate;
    }

    void set_post_validate_mode( PostValidate::Mode mode )
    {
        modes.post_validate = mode;
    }

    DelAttr::Mode get_delattr_mode()
    {
        return modes.delattr;
    }

    void set_delattr_mode( DelAttr::Mode mode )
    {
        modes.delattr = mode;
    }

    GetState::Mode get_getstate_mode()
    {
        return modes.getstate;
    }

    void set_getstate_mode( GetState::Mode mode )
    {
        modes.getstate = mode;
    }

    PyObject* getattr( CAtom* atom );

    int setattr( CAtom* atom, PyObject* value );

    int delattr( CAtom* atom );

    PyObject* post_getattr( CAtom* atom, PyObject* value );

    int post_setattr( CAtom* atom, PyObject* oldvalue, PyObject* newvalue );

    PyObject* default_value( CAtom* atom );

    PyObject* validate( CAtom* atom, PyObject* oldvalue, PyObject* newvalue );

    PyObject* post_validate( CAtom* atom, PyObject* oldvalue, PyObject* newvalue );

    PyObject* full_validate( CAtom* atom, PyObject* oldvalue, PyObject* newvalue );

    PyObject* should_getstate( CAtom* atom );

    bool has_observers()
    {
        return static_observers && static_observers->size() > 0;
    }

    bool has_observers( uint8_t change_types );

    bool has_observer( PyObject* observer )
    {
        return has_observer( observer, ChangeType::Any );
    }

    bool has_observer( PyObject* observer, uint8_t change_types );

    void add_observer( PyObject* observer )
    {
        return add_observer( observer, ChangeType::Any );
    }

    void add_observer( PyObject* observer, uint8_t change_types );

    void remove_observer( PyObject* observer );

    bool notify( CAtom* atom, PyObject* args, PyObject* kwargs )
    {
        return notify( atom, args, kwargs, ChangeType::Any );
    }

    bool notify( CAtom* atom, PyObject* args, PyObject* kwargs, uint8_t change_types );

    static bool check_context( GetAttr::Mode mode, PyObject* context );

    static bool check_context( PostGetAttr::Mode mode, PyObject* context );

    static bool check_context( SetAttr::Mode mode, PyObject* context );

    static bool check_context( PostSetAttr::Mode mode, PyObject* context );

    static bool check_context( DefaultValue::Mode mode, PyObject* context );

    static bool check_context( Validate::Mode mode, PyObject* context );

    static bool check_context( PostValidate::Mode mode, PyObject* context );

    static bool check_context( DelAttr::Mode mode, PyObject* context );

    static bool check_context( GetState::Mode mode, PyObject* context );

    static int TypeCheck( PyObject* object )
    {
        return PyObject_TypeCheck( object, TypeObject );
    }
};

}  // namespace atom
