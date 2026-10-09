/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2025, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file LICENSE, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once
#include <Python.h>

namespace atom
{

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
        // If an exception occurred we store it and restore it after the
        // modification have been done as otherwise it can (Python 3) cause
        // boolean tests (PyObject_IsTrue) to fail for wrong reasons.
        bool exception_set = false;
#if PY_VERSION_HEX >= 0x030D0000
        PyObject *exc;
        if( PyErr_Occurred() ){
            exc = PyErr_GetRaisedException();
            exception_set = true;
        }
#else
        PyObject *type, *value, *traceback;
        if( PyErr_Occurred() ){
            PyErr_Fetch(&type, &value, &traceback);
            exception_set = true;
        }
#endif

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

        // Restore previous exception if one was set.
        if( exception_set ) {
#if PY_VERSION_HEX >= 0x030D0000
            PyErr_SetRaisedException(exc);
#else
            PyErr_Restore(type, value, traceback);
#endif
        }
    }

    void add_task( ModifyTask* task ) { m_tasks.push_back( task ); }

private:

    _T& m_owner;
    std::vector<ModifyTask*> m_tasks;

};

}  // namespace atom
