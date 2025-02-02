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
#include "platstdint.h"
#include "observer.h"
#include "modifyguard.h"
#include "utils.h"

#define MAX_OBSERVER_POOL_COUNT ( static_cast<uint32_t>( 0xffffffff ) )

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

    bool has_guard()
    {
        return m_modify_guard != nullptr;
    }

    bool has_topic( cppy::ptr& topic );

    bool has_observer( cppy::ptr& topic, cppy::ptr& observer )
    {
        return has_observer( topic, observer, ChangeType::Any );
    }

    bool has_observer( cppy::ptr& topic, cppy::ptr& observer, uint8_t change_types );

    void add( cppy::ptr& topic, cppy::ptr& observer, uint8_t member_changes );

    void remove( cppy::ptr& topic, cppy::ptr& observer );

    void remove( cppy::ptr& topic );

    void clear();

    // Clear and release back into the pool manager after the guard is released
    void release( uint32_t pool_index );

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

private:

    ModifyGuard<ObserverPool>* m_modify_guard;
    std::vector<Topic> m_topics;
    std::vector<Observer> m_observers;
};


class ObserverPoolManager
{

public:
    static ObserverPoolManager* get();

    // Aquire a new ObserverPool. If no free spots are available, allocate a new spot
    bool acquire_pool(uint32_t &index);

    // Access a pool at the given index
    inline ObserverPool* access_pool(uint32_t index) {
        return m_pools.at(index);
    }

    // Release and free the pool at the given index
    void release_pool(uint32_t index);

    ObserverPoolManager() {}
    ~ObserverPoolManager() {}
private:
    std::vector<ObserverPool*> m_pools;
    std::vector<uint32_t> m_free_slots;
};

} // namespace atom
