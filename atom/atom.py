#------------------------------------------------------------------------------
# Copyright (c) 2014, Nucleic
#
# Distributed under the terms of the BSD 3-Clause License.
#
# The full license is in the file LICENSE, distributed with this software.
#------------------------------------------------------------------------------
from types import FunctionType

import six

from .catom import CAtom, CMember


def __newobj__(cls, *args):
    """ A compatibility pickler function.

    This function is not part of the public Atom api.

    """
    return cls.__new__(cls, *args)


class AtomMeta(type):
    """ The metaclass for classes derived from Atom.

    This metaclass computes the memory layout of the members in a given
    class so that the CAtom class can allocate exactly enough space for
    the object data slots when it instantiates an object. It also sets
    up the specially name handler methods defined on the class.

    """
    def __new__(meta, name, bases, dct):
        # Instance dicts are disabled unless explicitly requested.
        if '__slots__' not in dct:
            dct['__slots__'] = ()

        cls = type.__new__(meta, name, bases, dct)

        # Collect all attribute names which might be members
        # defined on the class or on any of its base classes.
        names = set(dct)
        for base in cls.__mro__[1:-1]:
            if base is not CAtom and issubclass(base, CAtom):
                names.update(base.__members__)

        # Pass over the names and lookup the class member objects.
        # Each member is cloned so that it can be modified without
        # changing the behavior of a base class. This is slightly
        # less efficient than computing which base class members
        # can be shared, but is much simpler to implement.
        members = {}
        for name in names:
            value = getattr(cls, name, None)
            if isinstance(value, CMember):
                members[name] = value.clone()

        # Apply the cloned members and give them a storage index.
        for index, (key, value) in enumerate(six.iteritems(members)):
            setattr(cls, key, value)
            value.index = index

        # Process the class dict for any specially named handlers.
        for key, value in six.iteritems(dct):
            if key.startswith('_default_'):
                name = key[9:]
                if name not in members:
                    continue
                if isinstance(value, FunctionType):
                    mode = (CMember.DefaultAtomMethod, key)
                    members[name].default_mode = mode
                else:
                    mode = (CMember.DefaultValue, value)
                    members[name].default_mode = mode
            elif key.startswith('_validate_'):
                name = key[10:]
                if name not in members:
                    continue
                if isinstance(value, FunctionType):
                    mode = (CMember.ValidateAtomMethod, key)
                    members[name].validate_mode = mode
            elif key.startswith('_post_validate_'):
                name = key[15:]
                if name not in members:
                    continue
                if isinstance(value, FunctionType):
                    mode = (CMember.PostValidateAtomMethod, key)
                    members[name].post_validate_mode = mode
            elif key.startswith('_post_setattr_'):
                name = key[14:]
                if name not in members:
                    continue
                if isinstance(value, FunctionType):
                    mode = (CMember.PostSetattrAtomMethod, key)
                    members[name].post_setattr_mode = mode

        # Set the tuple of member names for the class. It is used
        # used for Python-land introspection and by the C++ layer
        # to determine the allocation size for new instances.
        cls.__members__ = tuple(members)

        return cls


@six.add_metaclass(AtomMeta)
class Atom(CAtom):
    """ The base class for defining atom objects.

    Atom objects are special Python objects which never allocate an
    instance dictionary unless one is explicitly requested. The data
    storage for an atom instance is instead computed from the Member
    objects declared in the class body. Memory is reserved for these
    members with no over-allocation.

    This restriction makes atom objects slightly less flexible than
    regular Python objects, but they are 3x - 10x more memory efficient
    than normal objects, and are 10% - 20%  faster on attribute access.

    All classes deriving from Atom will be automatically slotted, which
    will prevent the creation of an instance dictionary. If an instance
    dict is required, then the subclass should define a __dict__ slot.

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
        for name in type(self).__members__:
            state[name] = getattr(self, name)
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
