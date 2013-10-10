/*-----------------------------------------------------------------------------
| Copyright (c) 2013, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once

#include "inttypes.h"
#include "pythonhelpers.h"
#include "observerpool.h"


#define MAX_MEMBER_COUNT ( static_cast<uint32_t>( 0xffff ) )
#define SLOT_COUNT_MASK ( static_cast<uint32_t>( 0xffff ) )
#define FLAGS_MASK ( static_cast<uint32_t>( 0xffff0000 ) )
#define NOTIFICATION_BIT ( static_cast<uint32_t>( 1 << 16 ) )
#define GUARD_BIT ( static_cast<uint32_t>( 1 << 17 ) )
#define ATOMREF_BIT ( static_cast<uint32_t>( 1 << 18 ) )
#define catom_cast( o ) ( reinterpret_cast<CAtom*>( o ) )


extern PyTypeObject CAtom_Type;


struct CAtom
{
    PyObject_HEAD
    uint32_t bitfield;  // lower 16 == slot count, upper 16 == flags
    PyObject** slots;
    ObserverPool* observers;

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
        return PythonHelpers::xnewref( slots[ index ] );
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
        return bool( bitfield & NOTIFICATION_BIT );
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
        return bool( bitfield & GUARD_BIT );
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
        return bool( bitfield & ATOMREF_BIT );
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
            PyObjectPtr topicptr( PythonHelpers::newref( topic ) );
            return observers->has_topic( topicptr );
        }
        return false;
    }

    bool observe( PyObject* topic, PyObject* callback );

    bool unobserve( PyObject* topic, PyObject* callback );

    bool unobserve( PyObject* topic );

    bool unobserve();

    bool notify( PyObject* topic, PyObject* args, PyObject* kwargs );

    static int TypeCheck( PyObject* object )
    {
        return PyObject_TypeCheck( object, &CAtom_Type );
    }

    static void add_guard( CAtom** ptr );

    static void remove_guard( CAtom** ptr );

    static void change_guard( CAtom** ptr, CAtom* o );

    static void clear_guards( CAtom* o );
};


int
import_catom();
