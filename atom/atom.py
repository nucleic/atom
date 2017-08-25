#------------------------------------------------------------------------------
# Copyright (c) 2013, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
from __future__ import (division, print_function, absolute_import)

import sys
if sys.version_info >= (3,):
    import copyreg as copy_reg
else:
    import copy_reg
from contextlib import contextmanager

from types import FunctionType
from future.utils import with_metaclass

from .catom import (
    CAtom, Member, DefaultValue, PostGetAttr, PostSetAttr, Validate,
    PostValidate,
)


OBSERVE_PREFIX = '_observe_'
DEFAULT_PREFIX = '_default_'
VALIDATE_PREFIX = '_validate_'
POST_GETATTR_PREFIX = '_post_getattr_'
POST_SETATTR_PREFIX = '_post_setattr_'
POST_VALIDATE_PREFIX = '_post_validate_'


def observe(*names):
    """ A decorator which can be used to observe members on a class.

    Parameters
    ----------
    *names
        The str names of the attributes to observe on the object.
        These must be of the form 'foo' or 'foo.bar'.

    """
    # backwards compatibility for a single tuple or list argument
    if len(names) == 1 and isinstance(names[0], (tuple, list)):
        names = names[0]
    pairs = []
    for name in names:
        if type(name) is not str:
            msg = "observe attribute name must be a string, got '%s' instead"
            raise TypeError(msg % type(name).__name__)
        ndots = name.count('.')
        if ndots > 1:
            msg = "cannot observe '%s', only a single extension is allowed"
            raise TypeError(msg % name)
        if ndots == 1:
            name, attr = name.split('.')
            pairs.append((name, attr))
        else:
            pairs.append((name, None))
    return ObserveHandler(pairs)


class ObserveHandler(object):
    """ An object used to temporarily store observe decorator state.

    """
    __slots__ = ('pairs', 'func', 'funcname')

    def __init__(self, pairs):
        """ Initialize an ObserveHandler.

        Parameters
        ----------
        pairs : list
            The list of 2-tuples which store the pair information
            for the observers.

        """
        self.pairs = pairs
        self.func = None        # set by the __call__ method
        self.funcname = None    # storage for the metaclass

    def __call__(self, func):
        """ Called to decorate the function.

        """
        assert isinstance(func, FunctionType), "func must be a function"
        self.func = func
        return self

    def clone(self):
        """ Create a clone of the sentinel.

        """
        clone = type(self)(self.pairs)
        clone.func = self.func
        return clone


class set_default(object):
    """ An object used to set the default value of a base class member.

    """
    __slots__ = ('value', 'name')

    def __init__(self, value):
        self.value = value
        self.name = None    # storage for the metaclass

    def clone(self):
        """ Create a clone of the sentinel.

        """
        return type(self)(self.value)


class ExtendedObserver(object):
    """ A callable object used to implement extended observers.

    """
    __slots__ = ('funcname', 'attr')

    def __init__(self, funcname, attr):
        """ Initialize an ExtendedObserver.

        Parameters
        ----------
        funcname : str
            The function name on the owner object which should be
            used as the observer.

        attr : str
            The attribute name on the target object which should be
            observed.

        """
        self.funcname = funcname
        self.attr = attr

    def __call__(self, change):
        """ Handle a change of the target object.

        This handler will remove the old observer and attach a new
        observer to the target attribute. If the target object is not
        an Atom object, an exception will be raised.

        """
        old = None
        new = None
        ctype = change['type']
        if ctype == 'create':
            new = change['value']
        elif ctype == 'update':
            old = change['oldvalue']
            new = change['value']
        elif ctype == 'delete':
            old = change['value']
        attr = self.attr
        owner = change['object']
        handler = getattr(owner, self.funcname)
        if isinstance(old, Atom):
            old.unobserve(attr, handler)
        if isinstance(new, Atom):
            new.observe(attr, handler)
        elif new is not None:
            msg = "cannot attach observer '%s' to non-Atom %s"
            raise TypeError(msg % (attr, new))


class AtomMeta(type):
    """ The metaclass for classes derived from Atom.

    This metaclass computes the memory layout of the members in a given
    class so that the CAtom class can allocate exactly enough space for
    the object data slots when it instantiates an object.

    All classes deriving from Atom will be automatically slotted, which
    will prevent the creation of an instance dictionary and also the
    ability of an Atom to be weakly referenceable. If that behavior is
    required, then a subclasss should declare the appropriate slots.

    """
    def __new__(meta, name, bases, dct):
        # Unless the developer requests slots, they are automatically
        # turned off. This prevents the creation of instance dicts and
        # other space consuming features unless explicitly requested.
        if '__slots__' not in dct:
            dct['__slots__'] = ()

        # Pass over the class dict once and collect the information
        # necessary to implement the various behaviors. Some objects
        # declared on the class only serve as sentinels, and they are
        # removed from the dict before creating the class.
        observes = []               # Static observer methods: _observe_*
        defaults = []               # Default value methods: _default_*
        validates = []              # Validator methods: _validate_*
        decorated = []              # Decorated observers: @observe
        set_defaults = []           # set_default() sentinel
        post_getattrs = []          # Post getattr methods: _post_getattr_*
        post_setattrs = []          # Post setattr methods: _post_setattr_*
        post_validates = []         # Post validate methods: _post_validate_*
        seen_sentinels = set()      # The set of seen sentinels
        seen_decorated = set()      # The set of seen @observe decorators
        for key, value in dct.items():
            if isinstance(value, set_default):
                if value in seen_sentinels:
                    value = value.clone()
                value.name = key
                set_defaults.append(value)
                seen_sentinels.add(value)
                continue
            if isinstance(value, ObserveHandler):
                if value in seen_decorated:
                    value = value.clone()
                seen_decorated.add(value)
                decorated.append(value)
                value.funcname = key
                value = value.func
                dct[key] = value
                # Coninue processing the unwrapped function
            if isinstance(value, FunctionType):
                if key.startswith(OBSERVE_PREFIX):
                    observes.append(key)
                elif key.startswith(DEFAULT_PREFIX):
                    defaults.append(key)
                elif key.startswith(VALIDATE_PREFIX):
                    validates.append(key)
                elif key.startswith(POST_VALIDATE_PREFIX):
                    post_validates.append(key)
                elif key.startswith(POST_GETATTR_PREFIX):
                    post_getattrs.append(key)
                elif key.startswith(POST_SETATTR_PREFIX):
                    post_setattrs.append(key)

        # Remove the sentinels from the dict before creating the class.
        # The sentinels for the @observe decorators are already removed.
        for s in seen_sentinels:
            del dct[s.name]

        # Create the class object.
        cls = type.__new__(meta, name, bases, dct)

        # Walk the mro of the class, excluding itself, in reverse order
        # collecting all of the members into a single dict. The reverse
        # update preserves the mro of overridden members.
        members = {}
        for base in reversed(cls.__mro__[1:-1]):
            if base is not CAtom and issubclass(base, CAtom):
                members.update(base.__atom_members__)

        # The set of members which live on this class as opposed to a
        # base class. This enables the code which hooks up the various
        # static behaviors to only clone a member when necessary.
        owned_members = set()

        # Resolve any conflicts with memory layout. Conflicts can occur
        # with multiple inheritance where the indices of multiple base
        # classes will overlap. When this happens, the members which
        # conflict must be cloned in order to occupy a unique index.
        conflicts = []
        occupied = set()
        for member in members.values():
            if member.index in occupied:
                conflicts.append(member)
            else:
                occupied.add(member.index)

        # Clone the conflicting members and give them a unique index.
        # Do not blow away an overridden item on the current class.
        resolved_index = len(occupied)
        for member in conflicts:
            clone = member.clone()
            clone.set_index(resolved_index)
            owned_members.add(clone)
            members[clone.name] = clone
            if clone.name not in dct:
                setattr(cls, clone.name, clone)
            resolved_index += 1

        # Walk the dict a second time to collect the class members. This
        # assigns the name and the index to the member. If a member is
        # overriding an existing member, the memory index of the old
        # member is reused and any static observers are copied over.
        for key, value in dct.items():
            if isinstance(value, Member):
                if value in owned_members:  # foo = bar = Baz()
                    value = value.clone()
                    setattr(cls, key, value)
                owned_members.add(value)
                value.set_name(key)
                if key in members:
                    supermember = members[key]
                    members[key] = value
                    value.set_index(supermember.index)
                    value.copy_static_observers(supermember)
                else:
                    value.set_index(len(members))
                    members[key] = value

        # Add the special statically defined behaviors for the members.
        # If the target member is defined on a subclass, it is cloned
        # so that the behavior of the subclass is not modified.

        def clone_if_needed(m):
            if m not in owned_members:
                m = m.clone()
                members[m.name] = m
                owned_members.add(m)
                setattr(cls, m.name, m)
            return m

        # set_default() sentinels
        for sd in set_defaults:
            if sd.name not in members:
                msg = "Invalid call to set_default(). '%s' is not a member "
                msg += "on the '%s' class."
                raise TypeError(msg % (sd.name, name))
            member = clone_if_needed(members[sd.name])
            member.set_default_value_mode(DefaultValue.Static, sd.value)

        # _default_* methods
        n = len(DEFAULT_PREFIX)
        for mangled in defaults:
            target = mangled[n:]
            if target in members:
                member = clone_if_needed(members[target])
                member.set_default_value_mode(DefaultValue.ObjectMethod,
                                              mangled)

        # _validate_* methods
        n = len(VALIDATE_PREFIX)
        for mangled in validates:
            target = mangled[n:]
            if target in members:
                member = clone_if_needed(members[target])
                member.set_validate_mode(Validate.ObjectMethod_OldNew, mangled)

        # _post_validate_* methods
        n = len(POST_VALIDATE_PREFIX)
        for mangled in post_validates:
            target = mangled[n:]
            if target in members:
                member = clone_if_needed(members[target])
                member.set_post_validate_mode(PostValidate.ObjectMethod_OldNew,
                                              mangled)

        # _post_getattr_* methods
        n = len(POST_GETATTR_PREFIX)
        for mangled in post_getattrs:
            target = mangled[n:]
            if target in members:
                member = clone_if_needed(members[target])
                member.set_post_getattr_mode(PostGetAttr.ObjectMethod_Value,
                                             mangled)

        # _post_setattr_* methods
        n = len(POST_SETATTR_PREFIX)
        for mangled in post_setattrs:
            target = mangled[n:]
            if target in members:
                member = clone_if_needed(members[target])
                member.set_post_setattr_mode(PostSetAttr.ObjectMethod_OldNew,
                                             mangled)

        # _observe_* methods
        n = len(OBSERVE_PREFIX)
        for mangled in observes:
            target = mangled[n:]
            if target in members:
                member = clone_if_needed(members[target])
                member.add_static_observer(mangled)

        # @observe decorated methods
        for handler in decorated:
            for name, attr in handler.pairs:
                if name in members:
                    member = clone_if_needed(members[name])
                    observer = handler.funcname
                    if attr is not None:
                        observer = ExtendedObserver(observer, attr)
                    member.add_static_observer(observer)

        # Put a reference to the members dict on the class. This is used
        # by CAtom to query for the members and member count as needed.
        cls.__atom_members__ = members

        return cls


def __newobj__(cls, *args):
    """ A compatibility pickler function.

    This function is not part of the public Atom api.

    """
    return cls.__new__(cls, *args)


class Atom(with_metaclass(AtomMeta, CAtom)):
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

    @classmethod
    def members(cls):
        """ Get the members dictionary for the type.

        Returns
        -------
        result : dict
            The dictionary of members defined on the class. User code
            should not modify the contents of the dict.

        """
        return cls.__atom_members__

    @contextmanager
    def suppress_notifications(self):
        """ Disable member notifications within in a context.

        Returns
        -------
        result : contextmanager
            A context manager which disables atom notifications for the
            duration of the context. When the context exits, the state
            is restored to its previous value.

        """
        old = self.set_notifications_enabled(False)
        yield
        self.set_notifications_enabled(old)

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
        for key, value in state.items():
            setattr(self, key, value)
