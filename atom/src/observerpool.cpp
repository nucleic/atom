/*-----------------------------------------------------------------------------
| Copyright (c) 2013, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include "observerpool.h"


namespace
{

struct BaseTask : public ModifyTask
{
    BaseTask( ObserverPool& pool, PyObjectPtr& topic, PyObjectPtr& observer ) :
        m_pool( pool ), m_topic( topic ), m_observer( observer ) {}
    ObserverPool& m_pool;
    PyObjectPtr m_topic;
    PyObjectPtr m_observer;
};


struct AddTask : public BaseTask
{
    AddTask( ObserverPool& pool, PyObjectPtr& topic, PyObjectPtr& observer ) :
        BaseTask( pool, topic, observer ) {}
    void run() { m_pool.add( m_topic, m_observer ); }
};


struct RemoveTask : public BaseTask
{
    RemoveTask( ObserverPool& pool, PyObjectPtr& topic, PyObjectPtr& observer ) :
        BaseTask( pool, topic, observer ) {}
    void run() { m_pool.remove( m_topic, m_observer ); }
};


struct RemoveTopicTask : ModifyTask
{
    RemoveTopicTask( ObserverPool& pool, PyObjectPtr& topic ) :
        m_pool( pool ), m_topic( topic ) {}
    void run() { m_pool.remove( m_topic ); }
    ObserverPool& m_pool;
    PyObjectPtr m_topic;
};

} // namespace


bool
ObserverPool::has_topic( PyObjectPtr& topic )
{
    std::vector<Topic>::iterator topic_it;
    std::vector<Topic>::iterator topic_end = m_topics.end();
    for( topic_it = m_topics.begin(); topic_it != topic_end; ++topic_it )
    {
        if( topic_it->match( topic ) )
            return true;
    }
    return false;
}


void
ObserverPool::add( PyObjectPtr& topic, PyObjectPtr& observer )
{
    if( m_modify_guard )
    {
        ModifyTask* task = new AddTask( *this, topic, observer );
        m_modify_guard->add_task( task );
        return;
    }
    uint32_t obs_offset = 0;
    std::vector<Topic>::iterator topic_it;
    std::vector<Topic>::iterator topic_end = m_topics.end();
    for( topic_it = m_topics.begin(); topic_it != topic_end; ++topic_it )
    {
        if( topic_it->match( topic ) )
        {
            std::vector<PyObjectPtr>::iterator obs_it;
            std::vector<PyObjectPtr>::iterator obs_end;
            std::vector<PyObjectPtr>::iterator obs_free;
            obs_it = m_observers.begin() + obs_offset;
            obs_end = obs_it + topic_it->m_count;
            obs_free = obs_end;
            for( ; obs_it != obs_end; ++obs_it )
            {
                if( *obs_it == observer || obs_it->richcompare( observer, Py_EQ ) )
                    return;
                if( !obs_it->is_true() )
                    obs_free = obs_it;
            }
            if( obs_free == obs_end )
            {
                m_observers.insert( obs_end, observer );
                ++topic_it->m_count;
            }
            else
                *obs_free = observer;
            return;
        }
        obs_offset += topic_it->m_count;
    }
    m_topics.push_back( Topic( topic, 1 ) );
    m_observers.push_back( observer );
}


void
ObserverPool::remove( PyObjectPtr& topic, PyObjectPtr& observer )
{
    if( m_modify_guard )
    {
        ModifyTask* task = new RemoveTask( *this, topic, observer );
        m_modify_guard->add_task( task );
        return;
    }
    uint32_t obs_offset = 0;
    std::vector<Topic>::iterator topic_it;
    std::vector<Topic>::iterator topic_end = m_topics.end();
    for( topic_it = m_topics.begin(); topic_it != topic_end; ++topic_it )
    {
        if( topic_it->match( topic ) )
        {
            std::vector<PyObjectPtr>::iterator obs_it;
            std::vector<PyObjectPtr>::iterator obs_end;
            obs_it = m_observers.begin() + obs_offset;
            obs_end = obs_it + topic_it->m_count;
            for( ; obs_it != obs_end; ++obs_it )
            {
                if( *obs_it == observer || obs_it->richcompare( observer, Py_EQ ) )
                {
                    m_observers.erase( obs_it );
                    if( ( --topic_it->m_count ) == 0 )
                        m_topics.erase( topic_it );
                    return;
                }
            }
            return;
        }
        obs_offset += topic_it->m_count;
    }
}


void
ObserverPool::remove( PyObjectPtr& topic )
{
    if( m_modify_guard )
    {
        ModifyTask* task = new RemoveTopicTask( *this, topic );
        m_modify_guard->add_task( task );
        return;
    }
    uint32_t obs_offset = 0;
    std::vector<Topic>::iterator topic_it;
    std::vector<Topic>::iterator topic_end = m_topics.end();
    for( topic_it = m_topics.begin(); topic_it != topic_end; ++topic_it )
    {
        if( topic_it->match( topic ) )
        {
            m_observers.erase(
                m_observers.begin() + obs_offset,
                m_observers.begin() + (obs_offset + topic_it->m_count)
            );
            m_topics.erase( topic_it );
            return;
        }
        obs_offset += topic_it->m_count;
    }
}


bool
ObserverPool::notify( PyObjectPtr& topic, PyObjectPtr& args, PyObjectPtr& kwargs )
{
    ModifyGuard<ObserverPool> guard( *this );
    uint32_t obs_offset = 0;
    std::vector<Topic>::iterator topic_it;
    std::vector<Topic>::iterator topic_end = m_topics.end();
    for( topic_it = m_topics.begin(); topic_it != topic_end; ++topic_it )
    {
        if( topic_it->match( topic ) )
        {
            std::vector<PyObjectPtr>::iterator obs_it;
            std::vector<PyObjectPtr>::iterator obs_end;
            obs_it = m_observers.begin() + obs_offset;
            obs_end = obs_it + topic_it->m_count;
            for( ; obs_it != obs_end; ++obs_it )
            {
                if( obs_it->is_true() )
                {
                    if( !obs_it->operator()( args, kwargs ) )
                        return false;
                }
                else
                {
                    ModifyTask* task = new RemoveTask( *this, topic, *obs_it );
                    m_modify_guard->add_task( task );
                }
            }
            return true;
        }
        obs_offset += topic_it->m_count;
    }
    return true;
}


int
ObserverPool::py_traverse( visitproc visit, void* arg )
{
    int vret;
    std::vector<Topic>::iterator topic_it;
    std::vector<Topic>::iterator topic_end = m_topics.end();
    for( topic_it = m_topics.begin(); topic_it != topic_end; ++topic_it )
    {
        vret = visit( topic_it->m_topic.get(), arg );
        if( vret )
            return vret;
    }
    std::vector<PyObjectPtr>::iterator obs_it;
    std::vector<PyObjectPtr>::iterator obs_end = m_observers.end();
    for( obs_it = m_observers.begin(); obs_it != obs_end; ++obs_it )
    {
        vret = visit( obs_it->get(), arg );
        if( vret )
            return vret;
    }
    return 0;
}
