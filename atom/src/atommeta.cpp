/*-----------------------------------------------------------------------------
| Copyright (c) 2014, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include "atommeta.h"
#include "atom.h"
#include "member.h"
#include "py23compat.h"

#include <cppy/cppy.h>
#include <vector>
#include <utility>


#define pytype_cast( op ) reinterpret_cast<PyTypeObject*>( op )
#define member_cast( op ) reinterpret_cast<Member*>( op )


namespace atom
{

namespace
{

PyObject* slots_str;
PyObject* empty_tuple;


/**
 * Ensure empty slots are added to the class dict.
 *
 * This prevents the creation of space-consuming instance dicts.
 */
bool ensure_slots( PyObject* dict )
{
	if( PyDict_GetItem( dict, slots_str ) )
	{
		cppy::type_error( "Atom classes cannot not declare slots" );
		return false;
	}
	return PyDict_SetItem( dict, slots_str, empty_tuple ) == 0;
}


/**
 * Create a new class object from the given meta information.
 *
 * This performs the equivalent of type.__new__(meta, name, bases, dict)
 */
PyObject* create_class( PyObject* meta, PyObject* name, PyObject* bases, PyObject* dict )
{
	cppy::ptr args( PyTuple_Pack( 3, name, bases, dict ) );
	if( !args )
	{
		return 0;
	}
	cppy::ptr cls( PyType_Type.tp_new( pytype_cast( meta ), args.get(), 0 ) );
	if( !cls )
	{
		return 0;
	}
	if( !PyType_Check( cls.get() ) )
	{
		return cppy::type_error( "atom meta created a non-type object" );
	}
	return cls.release();
}


/**
 * Collect all base class members into a single dict.
 *
 * This walk the mro of the class, excluding itself, in reverse order
 * collecting all of the members into a single dict. Collecting the
 * members in reverse preserves the mro of overridden members.
 */
PyObject* collect_base_class_members( PyObject* type )
{
	cppy::ptr members( PyDict_New() );
	if( !members )
	{
		return 0;
	}
	PyObject* mro = pytype_cast( type )->tp_mro;
	PyTypeObject* AtomType = &Atom::TypeObject;
	for( Py_ssize_t i = PyTuple_GET_SIZE( mro ) - 1; i > 0; --i )
	{
		PyTypeObject* base = pytype_cast( PyTuple_GET_ITEM( mro, i ) );
		if( base != AtomType && PyType_IsSubtype( base, AtomType ) )
		{
			cppy::ptr others( Atom::LookupMembers( base ) );
			if( !others || PyDict_Update( members.get(), others.get() ) < 0 )
			{
				return 0;
			}
		}
	}
	return members.release();
}


/**
 * A new class members to the given members dict.
 *
 * This walk sthe current class dict and adds the new members to the
 * members dict. The index of the new members are a computed at this
 * time since they are easily determined. Conflicts due to multiple
 * inheritance will be resolved at a later time.
 */
bool add_new_class_members( PyObject* members, PyObject* class_dict )
{
	PyObject* key;
	PyObject* value;
	Py_ssize_t pos = 0;
	Py_ssize_t count = PyDict_Size( members );
	while( PyDict_Next( class_dict, &pos, &key, &value ) )
	{
		if( !Member::TypeCheck( value ) )
		{
			continue;
		}
		if( !Py23Str_Check( key ) )
		{
			cppy::type_error( "metaclass dict has non-string member keys" );
			return false;
		}
		if( PyObject* existing = PyDict_GetItem( members, key ) )
		{
			member_cast( value )->setIndex( member_cast( existing )->index() );
		}
		else
		{
			member_cast( value )->setIndex( count++ );
		}
		if( PyDict_SetItem( members, key, value ) < 0 )
		{
			return false;
		}
	}
	return true;
}


/**
 * Ensure that the members have a unique monotonic memory layout.
 *
 * This will clone any member which has a value index which conflicts
 * with another member or is out of range for the layout. The cloned
 * members will be updated with a unique index.
 */
bool fixup_memory_layout( PyObject* members )
{
	// Members with indices which conflict with other members are
	// collected into this vector, then cloned and given a new index.
	typedef std::pair<cppy::ptr, cppy::ptr> pair_t;
	std::vector<pair_t> conflicts;

	// The set of all valid indices for this collection of members.
	// Indices are added to this set as they are claimed by a member.
	Py_ssize_t count = PyDict_Size( members );
	std::vector<bool> indices( count, false );

	// Pass over the members and claim the used indices. Any member
	// which conflicts with another is added to the conflicts vector.
	PyObject* key;
	PyObject* value;
	Py_ssize_t pos = 0;
	while( PyDict_Next( members, &pos, &key, &value ) )
	{
		Py_ssize_t index = member_cast( value )->index();
		if( index >= count || indices[ index ] )
		{
			pair_t pair( cppy::incref( key ), cppy::incref( value ) );
			conflicts.push_back( pair );
		}
		else
		{
			indices[ index ] = true;
		}
	}

	// For common cases (single-inheritance) there will be no conflicts.
	if( conflicts.size() == 0 )
	{
		return true;
	}

	// The unused indices are distributed among the conflicts. The
	// conflicting member is cloned as its index may be valid if it
	// belongs to a base class in a multiple inheritance hierarchy.
	Py_ssize_t conflict_index = 0;
	for( Py_ssize_t index = 0; index < count; ++index )
	{
		if( indices[ index ] )
		{
			continue;
		}
		pair_t& pair( conflicts[ conflict_index++ ] );
		cppy::ptr clone( Member::Clone( pair.second.get() ) );
		if( !clone )
		{
			return false;
		}
		member_cast( clone.get() )->setIndex( index );
		if( PyDict_SetItem( members, pair.first.get(), clone.get() ) < 0 )
		{
			return false;
		}
	}

	return true;
}

} // namespace


PyObject* AtomMeta::CreateClass( PyObject* args )
{
	PyObject* meta;
	PyObject* name;
	PyObject* bases;
	PyObject* dict;
	if( !PyArg_ParseTuple( args, "OOOO", &meta, &name, &bases, &dict ) )
	{
		return 0;
	}
	if( !PyType_Check( meta ) )
	{
		return cppy::type_error( meta, "type" );
	}
	if( !Py23Str_Check( name ) )
	{
		return cppy::type_error( name, "str" );
	}
	if( !PyTuple_Check( bases ) )
	{
		return cppy::type_error( bases, "tuple" );
	}
	if( !PyDict_Check( dict ) )
	{
		return cppy::type_error( dict, "dict" );
	}
	if( !ensure_slots( dict ) )
	{
		return 0;
	}
	cppy::ptr cls( create_class( meta, name, bases, dict ) );
	if( !cls )
	{
		return 0;
	}
	cppy::ptr members( collect_base_class_members( cls.get() ) );
	if( !members )
	{
		return 0;
	}
	if( !add_new_class_members( members.get(), dict ) )
	{
		return 0;
	}
	if( !fixup_memory_layout( members.get() ) )
	{
		return 0;
	}
	if( !Atom::RegisterMembers( pytype_cast( cls.get() ), members.get() ) )
	{
		return 0;
	}
	return cls.release();
}


bool AtomMeta::Ready()
{
	if( !( slots_str = Py23Str_FromString( "__slots__" ) ) )
	{
		return false;
	}
	if( !( empty_tuple = PyTuple_New( 0 ) ) )
	{
		return false;
	}
	return true;
}

} // namespace atom
