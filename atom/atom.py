#------------------------------------------------------------------------------
# Copyright (c) 2014, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
import copy_reg

from .catom import CAtom, CMember, _register_members, _lookup_members


def _uniquify_memory_layout(members):
    """ Ensure that the given members occupy a unique memory layout.

    This function will ensure that each member has a unique and
    monotonically increasing value index which corresponds to the
    memory slot index of the value in C++.

    This function operates on the members dict in-place.

    Parameters
    ----------
    members : dict
        A dict of member name -> member.

    """
    # Members with indices which conflict with other members are
    # collected into this list, then cloned and given a new index.
    conflicts = []

    # The set of all valid indices for this collection of members.
    # Indices are removed from this set as they are claimed.
    indices = set(range(len(members)))

    # Pass over the members and claim the used indices. Any member
    # which conflicts with another is added to the conflicts list.
    for name, member in members.iteritems():
        index = member._value_index
        if index in indices:
            indices.remove(index)
        else:
            conflicts.append((name, member))

    # The remaining indices are distributed among the conflicts. The
    # conflicting member is cloned since its index may be valid if it
    # belongs to a base class in a multiple inheritance hierarchy.
    for (name, member), index in zip(conflicts, indices):
        member = member.clone()
        member._value_index = index
        members[name] = member


class AtomMeta(type):
    """ The metaclass for classes derived from Atom.

    This metaclass computes the atom member layout for the class so
    that the CAtom class can allocate exactly enough space for the
    instance data slots when it instantiates an object.

    All classes deriving from Atom are automatically slotted.

    """
    def __new__(meta, name, bases, dct):
        # Unless the developer requests slots, they are automatically
        # turned off. This prevents the creation of instance dicts and
        # other space consuming features unless explicitly requested.
        if '__slots__' not in dct:
            dct['__slots__'] = ()

        # Create the class object.
        cls = type.__new__(meta, name, bases, dct)

        # Walk the mro of the class, excluding itself, in reverse order
        # collecting all of the members into a single dict. The reverse
        # update preserves the mro of overridden members.
        members = {}
        for base in reversed(cls.__mro__[1:-1]):
            if base is not CAtom and issubclass(base, CAtom):
                members.update(_lookup_members(base))

        # Walk the current class dict and collect the new members.
        # The index of the new members can be computed immediately.
        for key, value in dct.iteritems():
            if isinstance(value, CMember):
                if key in members:
                    value._value_index = members[key]._value_index
                else:
                    value._value_index = len(members)
                members[key] = value

        # Compute a unique memory layout for the members.
        _uniquify_memory_layout(members)

        # Register the final members with the C++ member registry.
        _register_members(cls, members)

        return cls


def __newobj__(cls, *args):
    """ A compatibility pickler function.

    This function is not part of the public Atom api.

    """
    return cls.__new__(cls, *args)


class Atom(CAtom):
    """ The base class for defining atom objects.

    `Atom` objects are special Python objects which never allocate an
    instance dictionary unless one is explicitly requested. The storage
    for an atom is instead computed from the `Member` objects declared
    on the class. Memory is reserved for these members with no over
    allocation.

    This restriction make atom objects a bit less flexible than normal
    Python objects, but they are between 3x-10x more memory efficient
    than normal objects depending on the number of attributes.

    """
    __metaclass__ = AtomMeta

    __slots__ = '__weakref__'

    def __reduce_ex__(self, proto):
        """ An implementation of the reduce protocol.

        This method creates a reduction tuple for Atom instances. This
        method should not be overridden by subclasses unless the author
        fully understands the rammifications.

        """
        args = (type(self),) + self.__getnewargs__()
        return (__newobj__, args, self.__getstate__())

    def __getnewargs__(self):
        """ Get the argument tuple to pass to __new__ on unpickling.

        See the Python.org docs for more information.

        """
        return ()

    def __getstate__(self):
        """ The base implementation of the pickle getstate protocol.

        This base class implementation handles the generic case where
        the object and all of its state are pickable. This includes
        state stored in Atom members, as well as any instance dict or
        slot attributes. Subclasses which require further customization
        should reimplement this method and modify the dict generated by
        this base class method.

        """
        state = {}
        state.update(getattr(self, '__dict__', {}))
        slots = copy_reg._slotnames(type(self))
        if slots:
            for name in slots:
                state[name] = getattr(self, name)
        for key in self.members():
            state[key] = getattr(self, key)
        return state

    def __setstate__(self, state):
        """ The base implementation of the pickle setstate protocol.

        This base class implementation handle the generic case of
        restoring an object using the state generated by the base
        class __getstate__ method. Subclasses which require custom
        behavior should reimplement this method.

        """
        for key, value in state.iteritems():
            setattr(self, key, value)
