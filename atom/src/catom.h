/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2019, Nucleic Development Team.
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
#define SLOT_COUNT_MASK ( static_cast<uint32_t>( 0xffff ) )
#define FLAGS_MASK ( static_cast<uint32_t>( 0xffff0000 ) )
#define NOTIFICATION_BIT ( static_cast<uint32_t>( 1 << 16 ) )
#define GUARD_BIT ( static_cast<uint32_t>( 1 << 17 ) )
#define ATOMREF_BIT ( static_cast<uint32_t>( 1 << 18 ) )
#define FROZEN_BIT ( static_cast<uint32_t>( 1 << 19 ) )
#define catom_cast( o ) ( reinterpret_cast<atom::CAtom*>( o ) )


namespace atom
{



struct CAtom
{
    PyObject_HEAD
    uint32_t bitfield;  // lower 16 == slot count, upper 16 == flags
    PyObject** slots;
    ObserverPool* observers;

    static PyType_Spec TypeObject_Spec;

    static PyTypeObject* TypeObject;

	static bool Ready();

    uint32_t get_slot_count()
    {
        return bitfield & SLOT_COUNT_MASK;
    }

    void set_slot_count( uint32_t count )
    {
        bitfield = ( bitfield & FLAGS_MASK ) | ( count & SLOT_COUNT_MASK );
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
        return ( bitfield & NOTIFICATION_BIT ) != 0;
    }

    void set_notifications_enabled( bool enabled )
    {
        if( enabled )
            bitfield |= NOTIFICATION_BIT;
        else
            bitfield &= ~NOTIFICATION_BIT;
    }

    bool has_guards()
    {
        return ( bitfield & GUARD_BIT ) != 0;
    }

    void set_has_guards( bool has_guards )
    {
        if( has_guards )
            bitfield |= GUARD_BIT;
        else
            bitfield &= ~GUARD_BIT;
    }

    bool has_atomref()
    {
        return ( bitfield & ATOMREF_BIT ) != 0;
    }

    void set_has_atomref( bool has_ref )
    {
        if( has_ref )
            bitfield |= ATOMREF_BIT;
        else
            bitfield &= ~ATOMREF_BIT;
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
        return ( bitfield & FROZEN_BIT ) != 0;
    }

    void set_frozen( bool frozen )
    {
        if( frozen )
            bitfield |= FROZEN_BIT;
        else
            bitfield &= ~FROZEN_BIT;
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
