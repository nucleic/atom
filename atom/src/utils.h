/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once
#include <Python.h>


namespace utils
{

inline bool
basestring_check( PyObject* obj )
{
    return (
        PyString_CheckExact( obj ) ||
        PyUnicode_CheckExact( obj ) ||
        PyObject_TypeCheck( obj, &PyBaseString_Type )
    );
}


template<typename T>
bool test_flag( T* object, typename T::Flag flag )
{
    return ( object->flags & static_cast<T::flags_t>( flag ) ) != 0;
}


template<typename T>
void set_flag( T* object, typename T::Flag flag, bool on=true )
{
    if( on )
    {
        object->flags |= static_cast<T::flags_t>( flag );
    }
    else
    {
        object->flags &= ~( static_cast<T::flags_t>( flag ) );
    }
}

} // namespace utils
