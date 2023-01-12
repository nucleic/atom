/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2017, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file LICENSE, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once

#include <vector>
#include <cppy/cppy.h>
#include "platstdint.h"
#include "observer.h"
#include "modifyguard.h"
#include "utils.h"


namespace atom
{


class ObserverPool
{

    struct Topic
    {
        Topic( cppy::ptr& topic ) : m_topic( topic ), m_count( 0 ) {}
        Topic( cppy::ptr& topic, uint32_t count ) : m_topic( topic ), m_count( count ) {}
        ~Topic() {}
        bool match( cppy::ptr& topic )
        {
            return m_topic == topic || utils::safe_richcompare( m_topic, topic, Py_EQ );
        }
        cppy::ptr m_topic;
        uint32_t m_count;
    };

    // ModifyGuard template interface
    friend class ModifyGuard<ObserverPool>;
    ModifyGuard<ObserverPool>* get_modify_guard() { return m_modify_guard; }
    void set_modify_guard( ModifyGuard<ObserverPool>* guard ) { m_modify_guard = guard; }

public:

    ObserverPool() : m_modify_guard( 0 ) {}

    ~ObserverPool() {}

    bool has_topic( cppy::ptr& topic );

    bool has_observer( cppy::ptr& topic, cppy::ptr& observer )
    {
        return has_observer( topic, observer, ChangeType::Any );
    }

    bool has_observer( cppy::ptr& topic, cppy::ptr& observer, uint8_t change_types );

    void add( cppy::ptr& topic, cppy::ptr& observer, uint8_t member_changes );

    void remove( cppy::ptr& topic, cppy::ptr& observer );

    void remove( cppy::ptr& topic );

    bool notify( cppy::ptr& topic, cppy::ptr& args, cppy::ptr& kwargs )
    {
        return notify( topic, args, kwargs, ChangeType::Any );
    }

    bool notify( cppy::ptr& topic, cppy::ptr& args, cppy::ptr& kwargs, uint8_t change_types );

    Py_ssize_t py_sizeof()
    {
        Py_ssize_t size = sizeof( ModifyGuard<ObserverPool>* );
        size += sizeof( std::vector<Topic> ) + sizeof( Topic ) * m_topics.capacity();
        size += sizeof( std::vector<Observer> ) + sizeof( Observer ) * m_observers.capacity();
        return size;
    };

    int py_traverse( visitproc visit, void* arg );

    void py_clear()
    {
        m_topics.clear();
        // Clearing the vector may cause arbitrary side effects on item
        // decref, including calls into methods which mutate the vector.
        // To avoid segfaults, first make the vector empty, then let the
        // destructors run for the old items.
        std::vector<Observer> empty;
        m_observers.swap( empty );
    }

private:

    ModifyGuard<ObserverPool>* m_modify_guard;
    std::vector<Topic> m_topics;
    std::vector<Observer> m_observers;
    ObserverPool(const ObserverPool& other);
    ObserverPool& operator=(const ObserverPool&);

};


} // namespace atom
