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

#include <utility>
#include <vector>


template <typename Key, typename T, typename Hash, typename Pred>
class SmallHash
{

public:

	typedef Key key_type;
	typedef T mapped_type;
	typedef std::pair<const Key, T> value_type;

	SmallHash( uint32_t size = 0 ) : m_data( alloc_size( size ) ) { }

private:

	struct Entry
	{
		enum State { Empty, Full, Deleted };
		value_type value;
		size_t hash;
		State state;
	};

	static uint32_t alloc_size( uint32_t n )
	{
	    static const uint32_t min_slots = 3;
	    static const uint32_t min_alloc = 4;
	    n = std::max( n, min_slots );
	    return next_power_of_2( n * min_alloc / min_slots );
	}

	std::vector<Entry> m_entries;
};
