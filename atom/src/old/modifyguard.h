/*-----------------------------------------------------------------------------
| Copyright (c) 2013, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once


struct ModifyTask
{
    ModifyTask() {}
    virtual ~ModifyTask() {}
    virtual void run() = 0;
};


template<class _T>
class ModifyGuard
{

public:

    ModifyGuard( _T& owner ) : m_owner( owner )
    {
        if( !m_owner.get_modify_guard() )
            m_owner.set_modify_guard( this );
    }

    ~ModifyGuard()
    {
        if( m_owner.get_modify_guard() == this )
        {
            m_owner.set_modify_guard( 0 );
            std::vector<ModifyTask*>::iterator it;
            std::vector<ModifyTask*>::iterator end = m_tasks.end();
            for( it = m_tasks.begin(); it != end; ++it )
            {
                ( *it )->run();
                delete *it;
            }
        }
    }

    void add_task( ModifyTask* task ) { m_tasks.push_back( task ); }

private:

    _T& m_owner;
    std::vector<ModifyTask*> m_tasks;

};
