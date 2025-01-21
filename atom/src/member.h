/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2024, Nucleic Development Team.
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
    uint8_t getattr: 4;
    uint8_t post_getattr: 3;
    uint8_t setattr: 4;
    uint8_t post_setattr: 3;
    uint8_t default_value: 4;
    uint8_t validate: 5;
    uint8_t post_validate: 3;
    uint8_t delattr: 3;
    uint8_t getstate: 3;
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
        return static_cast<GetAttr::Mode>( modes.getattr );
    }

    void set_getattr_mode( GetAttr::Mode mode )
    {
        modes.getattr = static_cast<uint8_t>( mode );
    }

    SetAttr::Mode get_setattr_mode()
    {
        return static_cast<SetAttr::Mode>( modes.setattr );
    }

    void set_setattr_mode( SetAttr::Mode mode )
    {
        modes.setattr = static_cast<uint8_t>( mode );
    }

    PostGetAttr::Mode get_post_getattr_mode()
    {
        return static_cast<PostGetAttr::Mode>( modes.post_getattr );
    }

    void set_post_getattr_mode( PostGetAttr::Mode mode )
    {
        modes.post_getattr = static_cast<uint8_t>( mode );
    }

    PostSetAttr::Mode get_post_setattr_mode()
    {
        return static_cast<PostSetAttr::Mode>( modes.post_setattr );
    }

    void set_post_setattr_mode( PostSetAttr::Mode mode )
    {
        modes.post_setattr = static_cast<uint8_t>( mode );
    }

    DefaultValue::Mode get_default_value_mode()
    {
        return static_cast<DefaultValue::Mode>( modes.default_value );
    }

    void set_default_value_mode( DefaultValue::Mode mode )
    {
        modes.default_value = static_cast<uint8_t>( mode );
    }

    Validate::Mode get_validate_mode()
    {
        return static_cast<Validate::Mode>( modes.validate );
    }

    void set_validate_mode( Validate::Mode mode )
    {
        modes.validate = static_cast<uint8_t>( mode );
    }

    PostValidate::Mode get_post_validate_mode()
    {
        return static_cast<PostValidate::Mode>( modes.post_validate );
    }

    void set_post_validate_mode( PostValidate::Mode mode )
    {
        modes.post_validate = static_cast<uint8_t>( mode );
    }

    DelAttr::Mode get_delattr_mode()
    {
        return static_cast<DelAttr::Mode>( modes.delattr );
    }

    void set_delattr_mode( DelAttr::Mode mode )
    {
        modes.delattr = static_cast<uint8_t>( mode );
    }

    GetState::Mode get_getstate_mode()
    {
        return static_cast<GetState::Mode>( modes.getstate );
    }

    void set_getstate_mode( GetState::Mode mode )
    {
        modes.getstate = static_cast<uint8_t>( mode );
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
