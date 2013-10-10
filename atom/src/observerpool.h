/*-----------------------------------------------------------------------------
| Copyright (c) 2013, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once

#include <vector>
#include "inttypes.h"
#include "pythonhelpers.h"
#include "modifyguard.h"


using PythonHelpers::PyObjectPtr;


class ObserverPool
{

    struct Topic
    {
        Topic( PyObjectPtr& topic ) : m_topic( topic ), m_count( 0 ) {}
        Topic( PyObjectPtr& topic, uint32_t count ) : m_topic( topic ), m_count( count ) {}
        ~Topic() {}
        bool match( PyObjectPtr& topic )
        {
            return m_topic == topic || m_topic.richcompare( topic, Py_EQ );
        }
        PyObjectPtr m_topic;
        uint32_t m_count;
    };

    // ModifyGuard template interface
    friend class ModifyGuard<ObserverPool>;
    ModifyGuard<ObserverPool>* get_modify_guard() { return m_modify_guard; }
    void set_modify_guard( ModifyGuard<ObserverPool>* guard ) { m_modify_guard = guard; }

public:

    ObserverPool() : m_modify_guard( 0 ) {}

    ~ObserverPool() {}

    bool has_topic( PyObjectPtr& topic );

    void add( PyObjectPtr& topic, PyObjectPtr& observer );

    void remove( PyObjectPtr& topic, PyObjectPtr& observer );

    void remove( PyObjectPtr& topic );

    bool notify( PyObjectPtr& topic, PyObjectPtr& args, PyObjectPtr& kwargs );

    Py_ssize_t py_sizeof()
    {
        Py_ssize_t size = sizeof( ModifyGuard<ObserverPool>* );
        size += sizeof( std::vector<Topic> ) + sizeof( Topic ) * m_topics.capacity();
        size += sizeof( std::vector<PyObjectPtr> ) + sizeof( PyObjectPtr ) * m_observers.capacity();
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
        std::vector<PyObjectPtr> empty;
        m_observers.swap( empty );
    }

private:

    ModifyGuard<ObserverPool>* m_modify_guard;
    std::vector<Topic> m_topics;
    std::vector<PyObjectPtr> m_observers;
    ObserverPool(const ObserverPool& other);
    ObserverPool& operator=(const ObserverPool&);

};
