/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include <Python.h>
#include "static_strings.h"


namespace StaticStrings
{

PyObject* ClassMap;

PyObject* Default;
PyObject* DefaultPrefix;

PyObject* PostSetattr;
PyObject* PostSetattrPrefix;

PyObject* Validate;
PyObject* ValidatePrefix;

}  // namespace StaticStrings


int
import_static_strings()
{
	StaticStrings::ClassMap = PyString_FromString( "_[class map]" );
    if( !StaticStrings::ClassMap )
    {
        return -1;
    }
    StaticStrings::Default = PyString_FromString( "default" );
    if( !StaticStrings::Default )
    {
        return -1;
    }
    StaticStrings::DefaultPrefix = PyString_FromString( "_default_" );
    if( !StaticStrings::DefaultPrefix )
    {
        return -1;
    }
    StaticStrings::PostSetattr = PyString_FromString( "post_setattr" );
    if( !StaticStrings::PostSetattr )
    {
        return -1;
    }
    StaticStrings::PostSetattrPrefix = PyString_FromString( "_post_setattr_" );
    if( !StaticStrings::PostSetattrPrefix )
    {
        return -1;
    }
    StaticStrings::Validate = PyString_FromString( "validate" );
    if( !StaticStrings::Validate )
    {
        return -1;
    }
    StaticStrings::ValidatePrefix = PyString_FromString( "_validate_" );
    if( !StaticStrings::ValidatePrefix )
    {
        return -1;
    }
    return 0;
}
