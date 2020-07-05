/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2019, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file LICENSE, distributed with this software.
|----------------------------------------------------------------------------*/
#include "observerpool.h"
#include "utils.h"


namespace atom
{

namespace
{

struct BaseTask : public ModifyTask
{
    BaseTask( ObserverPool& pool, cppy::ptr& topic, cppy::ptr& observer ) :
        m_pool( pool ), m_topic( topic ), m_observer( observer ) {}
    ObserverPool& m_pool;
    cppy::ptr m_topic;
    cppy::ptr m_observer;
};


struct AddTask : public BaseTask
{
    AddTask( ObserverPool& pool, cppy::ptr& topic, cppy::ptr& observer ) :
        BaseTask( pool, topic, observer ) {}
    void run() { m_pool.add( m_topic, m_observer ); }
};


struct RemoveTask : public BaseTask
{
    RemoveTask( ObserverPool& pool, cppy::ptr& topic, cppy::ptr& observer ) :
        BaseTask( pool, topic, observer ) {}
    void run() { m_pool.remove( m_topic, m_observer ); }
};


struct RemoveTopicTask : ModifyTask
{
    RemoveTopicTask( ObserverPool& pool, cppy::ptr& topic ) :
        m_pool( pool ), m_topic( topic ) {}
    void run() { m_pool.remove( m_topic ); }
    ObserverPool& m_pool;
    cppy::ptr m_topic;
};

} // namespace


bool
ObserverPool::has_topic( cppy::ptr& topic )
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


bool
ObserverPool::has_observer( cppy::ptr& topic, cppy::ptr& observer )
{
    uint32_t obs_offset = 0;
    std::vector<Topic>::iterator topic_it;
    std::vector<Topic>::iterator topic_end = m_topics.end();
    for( topic_it = m_topics.begin(); topic_it != topic_end; ++topic_it )
    {
        if( topic_it->match( topic ) )
        {
            std::vector<cppy::ptr>::iterator obs_it;
            std::vector<cppy::ptr>::iterator obs_end;
            obs_it = m_observers.begin() + obs_offset;
            obs_end = obs_it + topic_it->m_count;
            for( ; obs_it != obs_end; ++obs_it )
            {
                if( *obs_it == observer || utils::safe_richcompare( obs_it->get(), observer, Py_EQ ) )
                    return true;
            }
            return false;
        }
        obs_offset += topic_it->m_count;
    }
    return false;
}


void
ObserverPool::add( cppy::ptr& topic, cppy::ptr& observer )
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
            std::vector<cppy::ptr>::iterator obs_it;
            std::vector<cppy::ptr>::iterator obs_end;
            std::vector<cppy::ptr>::iterator obs_free;
            obs_it = m_observers.begin() + obs_offset;
            obs_end = obs_it + topic_it->m_count;
            obs_free = obs_end;
            for( ; obs_it != obs_end; ++obs_it )
            {
                if( *obs_it == observer || utils::safe_richcompare( obs_it->get(), observer, Py_EQ ) )
                    return;
                if( !obs_it->is_truthy() )
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
ObserverPool::remove( cppy::ptr& topic, cppy::ptr& observer )
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
            std::vector<cppy::ptr>::iterator obs_it;
            std::vector<cppy::ptr>::iterator obs_end;
            obs_it = m_observers.begin() + obs_offset;
            obs_end = obs_it + topic_it->m_count;
            for( ; obs_it != obs_end; ++obs_it )
            {
                if( *obs_it == observer || utils::safe_richcompare( obs_it->get(), observer, Py_EQ ) )
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
ObserverPool::remove( cppy::ptr& topic )
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
ObserverPool::notify( cppy::ptr& topic, cppy::ptr& args, cppy::ptr& kwargs )
{
    ModifyGuard<ObserverPool> guard( *this );
    uint32_t obs_offset = 0;
    std::vector<Topic>::iterator topic_it;
    std::vector<Topic>::iterator topic_end = m_topics.end();
    for( topic_it = m_topics.begin(); topic_it != topic_end; ++topic_it )
    {
        if( topic_it->match( topic ) )
        {
            std::vector<cppy::ptr>::iterator obs_it;
            std::vector<cppy::ptr>::iterator obs_end;
            obs_it = m_observers.begin() + obs_offset;
            obs_end = obs_it + topic_it->m_count;
            for( ; obs_it != obs_end; ++obs_it )
            {
                if( obs_it->is_truthy() )
                {
                    if( !obs_it->call( args, kwargs ) )
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
    std::vector<cppy::ptr>::iterator obs_it;
    std::vector<cppy::ptr>::iterator obs_end = m_observers.end();
    for( obs_it = m_observers.begin(); obs_it != obs_end; ++obs_it )
    {
        vret = visit( obs_it->get(), arg );
        if( vret )
            return vret;
    }
    return 0;
}


}  //namespace atom
