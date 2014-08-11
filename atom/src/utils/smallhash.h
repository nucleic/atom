/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once

#include <utils/math.h>
#include <utils/stdint.h>

#include <vector>


template <typename Key, typename Value, typename Hash, typename Equal>
class SmallHash
{

public:

	typedef Key key_type;
	typedef Value value_type;

	SmallHash( uint32_t size = 0 ) : m_data( allocSize( size ) ) { }

	Value* find( const Key& key )
	{

	}

	void insert( const Key& key, const Value& value )
	{

	}

	void remove( const Key& key )
	{

	}

private:

	struct Entry
	{
		enum State { Empty, Full, Deleted };
		Key key;
		Value value;
		size_t hash;
		State state;
	};

	static uint32_t allocSize( uint32_t n )
	{
	    static const uint32_t min_slots = 3;
	    static const uint32_t min_alloc = 4;
	    n = std::max( n, min_slots );
	    return next_power_of_2( n * min_alloc / min_slots );
	}

	Entry* locate( const Key& key )
	{
		size_t hash = Hash()( key );

	}

	std::vector<Entry> m_entries;
};
