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
    AddTask( ObserverPool& pool, cppy::ptr& topic, cppy::ptr& observer, uint8_t change_types ) :
        BaseTask( pool, topic, observer ), m_change_types(change_types) {}
    void run() { m_pool.add( m_topic, m_observer, m_change_types ); }
    uint8_t m_change_types;
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
ObserverPool::has_observer( cppy::ptr& topic, cppy::ptr& observer, uint8_t change_types )
{
    uint32_t obs_offset = 0;
    std::vector<Topic>::iterator topic_it;
    std::vector<Topic>::iterator topic_end = m_topics.end();
    for( topic_it = m_topics.begin(); topic_it != topic_end; ++topic_it )
    {
        if( topic_it->match( topic ) )
        {
            std::vector<Observer>::iterator obs_it;
            std::vector<Observer>::iterator obs_end;
            obs_it = m_observers.begin() + obs_offset;
            obs_end = obs_it + topic_it->m_count;
            for( ; obs_it != obs_end; ++obs_it )
            {
                if( obs_it->match( observer ) && obs_it->enabled( change_types ) )
                    return true;
            }
            return false;
        }
        obs_offset += topic_it->m_count;
    }
    return false;
}


void
ObserverPool::add( cppy::ptr& topic, cppy::ptr& observer, uint8_t change_types )
{
    if( m_modify_guard )
    {
        ModifyTask* task = new AddTask( *this, topic, observer, change_types );
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
            std::vector<Observer>::iterator obs_it;
            std::vector<Observer>::iterator obs_end;
            std::vector<Observer>::iterator obs_free;
            obs_it = m_observers.begin() + obs_offset;
            obs_end = obs_it + topic_it->m_count;
            obs_free = obs_end;
            for( ; obs_it != obs_end; ++obs_it )
            {
                if( obs_it->match( observer ) )
                {
                    obs_it->m_change_types = change_types;
                    return;
                }
                if( !obs_it->m_observer.is_truthy() )
                    obs_free = obs_it;
            }
            if( obs_free == obs_end )
            {
                m_observers.insert( obs_end, Observer( observer, change_types ) );
                ++topic_it->m_count;
            }
            else
                *obs_free = Observer( observer, change_types );
            return;
        }
        obs_offset += topic_it->m_count;
    }
    m_topics.push_back( Topic( topic, 1 ) );
    m_observers.push_back( Observer(observer, change_types) );
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
            std::vector<Observer>::iterator obs_it;
            std::vector<Observer>::iterator obs_end;
            obs_it = m_observers.begin() + obs_offset;
            obs_end = obs_it + topic_it->m_count;
            for( ; obs_it != obs_end; ++obs_it )
            {
                if( obs_it->match( observer ) )
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
ObserverPool::notify( cppy::ptr& topic, cppy::ptr& args, cppy::ptr& kwargs, uint8_t change_types )
{
    ModifyGuard<ObserverPool> guard( *this );
    uint32_t obs_offset = 0;
    std::vector<Topic>::iterator topic_it;
    std::vector<Topic>::iterator topic_end = m_topics.end();
    for( topic_it = m_topics.begin(); topic_it != topic_end; ++topic_it )
    {
        if( topic_it->match( topic ) )
        {
            std::vector<Observer>::iterator obs_it;
            std::vector<Observer>::iterator obs_end;
            obs_it = m_observers.begin() + obs_offset;
            obs_end = obs_it + topic_it->m_count;
            for( ; obs_it != obs_end; ++obs_it )
            {
                if( obs_it->m_observer.is_truthy() )
                {
                    if( obs_it->enabled( change_types ) && !obs_it->m_observer.call( args, kwargs ) )
                        return false;
                }
                else
                {
                    ModifyTask* task = new RemoveTask( *this, topic, obs_it->m_observer );
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
    std::vector<Observer>::iterator obs_it;
    std::vector<Observer>::iterator obs_end = m_observers.end();
    for( obs_it = m_observers.begin(); obs_it != obs_end; ++obs_it )
    {
        vret = visit( obs_it->m_observer.get(), arg );
        if( vret )
            return vret;
    }
    return 0;
}


}  //namespace atom
