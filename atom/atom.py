#------------------------------------------------------------------------------
# Copyright (c) 2014, Nucleic
#
# Distributed under the terms of the BSD 3-Clause License.
#
# The full license is in the file LICENSE, distributed with this software.
#------------------------------------------------------------------------------
import six

from .catom import CAtom, CMember, _fp_lookup_members, _fp_register_members


def __newobj__(cls, *args):
    """ A compatibility pickler function.

    This function is not part of the public Atom api.

    """
    return cls.__new__(cls, *args)


def _remove_members(dct):
    """ Rempve the members from the given dict.

    This function is not part of the public Atom api.

    """
    # This walks the dict and removes all CMember instances.
    # The return value is a dict of the removed members.
    members = {}
    for key, value in six.iteritems(dct):
        if isinstance(value, CMember):
            members[key] = value
    for key in members:
        del dct[key]
    return members


def _add_base_class_members(members, cls):
    """ Collect the base class members for an Atom class.

    This function is not part of the public Atom api.

    """
    # This walk the mro of the class, excluding itself, in reverse
    # order collecting all of the members into the given dict. The
    # reverse order preserves the mro of overridden members.
    for base in reversed(cls.__mro__[1:-1]):
        if base is not CAtom and issubclass(base, CAtom):
            members.update(_fp_lookup_members(base))


def _add_new_class_members(members, cls_members):
    """ Add the new class members to a members dict.

    This function is not part of the public Atom api.

    """
    # This walks the class members dict and adds the new members to
    # the members dict. The index of the new members are a computed
    # at this time since they are easily determined. Conflicts due
    # to multiple inheritance will be resolved at a later time.
    next_index = len(members)
    for key, value in six.iteritems(cls_members):
        if key in members:
            value._fp_index = members[key]._fp_index
        else:
            value._fp_index = next_index
            next_index += 1
        members[key] = value


def _fixup_memory_layout(members):
    """ Fixup the memory layout for the given members.

    This function is not part of the public Atom api.

    """
    # Pass over the members and collect the used indices. Any member
    # which conflicts or is out-of-bounds is add to the conflicts.
    conflicts = []
    count = len(members)
    occupied = [False] * count
    for key, value in six.iteritems(members):
        index = value._fp_index
        if (index >= count or occupied[index]):
            conflicts.append((key, value))
        else:
            occupied[index] = True

    # The common case of single inheritance will have no conflicts.
    if not conflicts:
        return

    # The unused indices are distributed among the conflicts. The
    # conflicting member is cloned since it could be valid if it
    # belongs to base class in a multiple inheritance hierarchy.
    conflict_index = 0
    for index, flag in enumerate(occupied):
        if flag:
            continue
        key, value = conflicts[conflict_index]
        member = value.clone()
        member._fp_index = index
        members[key] = member
        conflict_index += 1


class AtomMeta(type):
    """ The metaclass for classes derived from Atom.

    This metaclass computes the atom member layout for the class so
    that the CAtom class can allocate exactly enough space for the
    instance data slots when it instantiates an object.

    All classes deriving from Atom are automatically slotted.

    """
    #__new__ = _atom_meta_create_class
    def __new__(meta, name, bases, dct):
        """ Create a new Atom class.

        Parameters
        ----------
        name : str
            The name of the class.

        bases : tuple
            The tuple of base classes.

        dct : dict
            The class dict.

        Returns
        -------
        result : type
            The new Atom class.

        """
        if '__slots__' in dct:
            raise TypeError('Atom classes cannot declare slots')
        cls_members = _remove_members(dct)
        cls = type.__new__(meta, name, bases, dct)
        members = {}
        _add_base_class_members(members, cls)
        _add_new_class_members(members, cls_members)
        _fixup_memory_layout(members)
        _fp_register_members(cls, members)
        return cls


@six.add_metaclass(AtomMeta)
class Atom(CAtom):
    """ The base class for defining atom objects.

    Atom objects are special Python objects which never allocate an
    instance dictionary unless one is explicitly requested. The data
    storage for an atom instance is instead computed from the Member
    objects declared in the class body. Memory is reserved for these
    members with no over-allocation.

    This restriction make atom objects a bit less flexible than normal
    Python objects, but they are 3x - 10x more memory efficient than
    normal objects, and are 10% - 20%  faster on attribute access.

    """
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
        the object and all of its state are pickable. Subclasses which
        require custom behavior should reimplement this method.

        """
        state = {}
        for key in self.get_members():
            state[key] = getattr(self, key)
        return state

    def __setstate__(self, state):
        """ The base implementation of the pickle setstate protocol.

        This base class implementation handle the generic case of
        restoring an object using the state generated by the base
        class __getstate__ method. Subclasses which require custom
        behavior should reimplement this method.

        """
        for key, value in six.iteritems(state):
            setattr(self, key, value)
