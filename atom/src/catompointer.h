/*-----------------------------------------------------------------------------
| Copyright (c) 2013-2017, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file LICENSE, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once
#include "catom.h"


namespace atom
{

// Shamelessly derived from qpointer.h
class CAtomPointer
{

public:

    inline CAtomPointer() : o( 0 ) {}

    inline CAtomPointer( CAtom* p ) : o( p )
    {
        CAtom::add_guard( &o );
    }

    inline CAtomPointer( const CAtomPointer& p ) : o( p.o )
    {
        CAtom::add_guard( &o );
    }

    inline ~CAtomPointer()
    {
        CAtom::remove_guard( &o );
    }

    inline CAtomPointer& operator=( const CAtomPointer &p )
    {
        if( this != &p )
            CAtom::change_guard( &o, p.o );
        return *this;
    }

    inline CAtomPointer& operator=( CAtom* p )
    {
        if( o != p )
            CAtom::change_guard( &o, p );
        return *this;
    }

    inline bool is_null() const
    {
        return !o;
    }

    inline CAtom* operator->() const
    {
        return const_cast<CAtom*>( o );
    }

    inline CAtom& operator*() const
    {
        return *const_cast<CAtom*>( o );
    }

    inline operator CAtom*() const
    {
        return const_cast<CAtom*>( o );
    }

    inline CAtom* data() const
    {
        return const_cast<CAtom*>( o );
    }

private:

    CAtom* o;
};

}  // namespace atom
