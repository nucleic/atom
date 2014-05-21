/*-----------------------------------------------------------------------------
| Copyright (c) 2013, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once

#include <Python.h>
#include "inttypes.h"
#include "pythonhelpers.h"
#include "observerpool.h"


int import_catom();


extern PyTypeObject CAtom_Type;


struct CAtom
{
    PyObject_HEAD
    PyObject** m_slots;
    ObserverPool* observers;
    uint16_t m_flags;
    uint16_t slot_count;

    enum Flag
    {
        NotificationsEnabled = 0x1,
        HasGuards = 0x2,
        HasAtomRef = 0x4,
        IsFrozen = 0x8,
    };

    static const uint32_t MaxMemberCount = 0xffff;

    static int type_check( PyObject* object );

    static void add_guard( CAtom** ptr );

    static void remove_guard( CAtom** ptr );

    static void change_guard( CAtom** ptr, CAtom* o );

    static void clear_guards( CAtom* o );

    PyObject* get_slot( uint32_t index );

    void set_slot( uint32_t index, PyObject* object );

    bool test_flag( Flag flag );

    void set_flag( Flag flag, bool on=true );

    bool has_observers( PyObject* topic );

    bool has_observer( PyObject* topic, PyObject* callback );

    bool observe( PyObject* topic, PyObject* callback );

    bool unobserve( PyObject* topic, PyObject* callback );

    bool unobserve( PyObject* topic );

    bool unobserve();

    bool notify( PyObject* topic, PyObject* args, PyObject* kwargs );
};


inline int
CAtom::type_check( PyObject* object )
{
    return PyObject_TypeCheck( object, &CAtom_Type );
}


inline bool
CAtom::test_flag( Flag flag )
{
    return ( m_flags & static_cast<uint16_t>( flag ) ) != 0;
}


inline void
CAtom::set_flag( Flag flag, bool on )
{
    if( on )
        m_flags |= static_cast<uint16_t>( flag );
    else
        m_flags &= ~( static_cast<uint16_t>( flag ) );
}


inline PyObject*
CAtom::get_slot( uint32_t index )
{
    return PythonHelpers::xnewref( m_slots[ index ] );
}


inline void
CAtom::set_slot( uint32_t index, PyObject* object )
{
    PyObject* old = m_slots[ index ];
    m_slots[ index ] = object;
    Py_XINCREF( object );
    Py_XDECREF( old );
}


inline bool
CAtom::has_observers( PyObject* topic )
{
    if( observers )
    {
        PyObjectPtr topicptr( PythonHelpers::newref( topic ) );
        return observers->has_topic( topicptr );
    }
    return false;
}


inline bool
CAtom::has_observer( PyObject* topic, PyObject* callback )
{
    if( observers )
    {
        PyObjectPtr topicptr( PythonHelpers::newref( topic ) );
        PyObjectPtr callbackptr( PythonHelpers::newref( callback ) );
        return observers->has_observer( topicptr, callbackptr );
    }
    return false;
}
