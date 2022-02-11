/*-----------------------------------------------------------------------------
| Copyright (c) 2022, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file LICENSE, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once

#include <cppy/cppy.h>
#include "utils.h"


namespace atom
{


namespace MemberChange
{

enum Type {
    Created = 1,
    Updated = 2,
    Deleted = 4,
    Event = 8,
    Property = 16,
    Container = 32,
    Any = 0xFF,
};

} // end memberchange


struct Observer
{

    Observer( cppy::ptr& observer ) : m_observer( observer ), m_change_types( 0xFF ) {}
    Observer( cppy::ptr& observer, uint8_t change_types ) : m_observer( observer ), m_change_types( change_types ) {}
    ~Observer() {}

    bool match(const cppy::ptr& observer ) const
    {
        return m_observer == observer || utils::safe_richcompare( m_observer, observer, Py_EQ );
    }

    inline bool enabled( uint8_t change_types ) const
    {
        return (m_change_types & change_types) != 0;
    }

    cppy::ptr m_observer;
    uint8_t m_change_types;

};

} // namespace atom
