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
        return bitfield & 0xffff;
    }

    void set_slot_count( uint32_t count )
    {
        bitfield |= ( count & 0xffff );
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
        return bool( ( bitfield >> 16 ) & 1 );
    }

    void set_notifications_enabled( bool enabled )
    {
        if( enabled )
            bitfield |= ( 1 << 16 );
        else
            bitfield &= ~( 1 << 16 );
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

    bool notify( PyObject* topic, PyObject* args, PyObject* kwargs );

    static int TypeCheck( PyObject* object )
    {
        return PyObject_TypeCheck( object, &CAtom_Type );
    }
};


int
import_catom();
