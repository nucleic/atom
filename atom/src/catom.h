/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2024, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file LICENSE, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once

#include <cppy/cppy.h>
#include "platstdint.h"
#include "observerpool.h"


#define MAX_MEMBER_COUNT ( static_cast<uint32_t>( 0xffff ) )
#define catom_cast( o ) ( reinterpret_cast<atom::CAtom*>( o ) )


namespace atom
{

PACK(struct CAtomInfo {
    uint16_t slot_count;
    bool notifications_enabled: 1;
    bool has_guards: 1;
    bool has_atomref: 1;
    bool is_frozen: 1;
    uint16_t reserved: 12;
});

struct CAtom
{
    PyObject_HEAD
    PyObject** slots;
    ObserverPool* observers;
    CAtomInfo info;

    static PyType_Spec TypeObject_Spec;

    static PyTypeObject* TypeObject;

	static bool Ready();

    uint32_t get_slot_count()
    {
        return info.slot_count;
    }

    void set_slot_count( uint16_t count )
    {
        info.slot_count = count;
    }

    PyObject* get_slot( uint32_t index )
    {
        return cppy::xincref( slots[ index ] );
    }

    void set_slot( uint32_t index, PyObject* object )
    {
        PyObject* old = slots[ index ];
        slots[ index ] = object;
        Py_XINCREF( object );
        Py_XDECREF( old );
    }

    bool get_notifications_enabled()
    {
        return info.notifications_enabled;
    }

    void set_notifications_enabled( bool enabled )
    {
        info.notifications_enabled = enabled;
    }

    bool has_guards()
    {
        return info.has_guards;
    }

    void set_has_guards( bool has_guards )
    {
        info.has_guards = has_guards;
    }

    bool has_atomref()
    {
        return info.has_atomref;
    }

    void set_has_atomref( bool has_ref )
    {
        info.has_atomref = has_ref;
    }

    bool has_observers( PyObject* topic )
    {
        if( observers )
        {
            cppy::ptr topicptr( cppy::incref( topic ) );
            return observers->has_topic( topicptr );
        }
        return false;
    }

    bool has_observer( PyObject* topic, PyObject* callback )
    {
        if( observers )
        {
            cppy::ptr topicptr( cppy::incref( topic ) );
            cppy::ptr callbackptr( cppy::incref( callback ) );
            return observers->has_observer( topicptr, callbackptr );
        }
        return false;
    }

    bool is_frozen()
    {
        return info.is_frozen;
    }

    void set_frozen( bool frozen )
    {
        info.is_frozen = frozen;
    }

    bool observe( PyObject* topic, PyObject* callback )
    {
        return observe( topic, callback, ChangeType::Any );
    }

    bool observe( PyObject* topic, PyObject* callback, uint8_t change_types );

    bool unobserve( PyObject* topic, PyObject* callback );

    bool unobserve( PyObject* topic );

    bool unobserve();

    bool notify( PyObject* topic, PyObject* args, PyObject* kwargs )
    {
        return notify( topic, args, kwargs, ChangeType::Any );
    }

    bool notify( PyObject* topic, PyObject* args, PyObject* kwargs, uint8_t change_types );

    static int TypeCheck( PyObject* object )
    {
        return PyObject_TypeCheck( object, TypeObject );
    }

    static void add_guard( CAtom** ptr );

    static void remove_guard( CAtom** ptr );

    static void change_guard( CAtom** ptr, CAtom* o );

    static void clear_guards( CAtom* o );
};


}  // namespace atom
