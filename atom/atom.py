#------------------------------------------------------------------------------
# Copyright (c) 2013, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
from contextlib import contextmanager
from types import FunctionType

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


class observe(object):
    """ A decorator which can be used to observe members on a class.

    class Foo(Atom)

        a = Member()
        b = Member()

        @observe('a')
        def printer(self, change):
            print change

        @observe(('a', 'b'))
        def printer2(self, change):
            print change

    """
    __slots__ = ('name', 'func', 'mangled')

    def __init__(self, name):
        """ Initialize an observe decorator.

        Parameters
        ----------
        name : str or iterable
            The name or iterable of names of members to observe on
            the atom.

        """
        self.name = name
        self.func = None          # set by the __call__ method
        self.mangled = None       # storage for the metaclass

    def __call__(self, func):
        """ Called to decorate the function.

        """
        assert isinstance(func, FunctionType), "func must be a function"
        self.func = func
        return self

    def clone(self):
        """ Create a clone of the sentinel.

        """
        clone = type(self)(self.name)
        clone.func = self.func
        return clone


class set_default(object):
    """ An object used to set the default value of a base class member.

    """
    __slots__ = ('value', 'name')

    def __init__(self, value):
        self.value = value
        self.name = None  # storage for the metaclass

    def clone(self):
        """ Create a clone of the sentinel.

        """
        return type(self)(self.value)


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
        funcs = []                  # Methods defined on the class
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
        for key, value in dct.iteritems():
            if isinstance(value, set_default):
                if value in seen_sentinels:
                    value = value.clone()
                value.name = key
                set_defaults.append(value)
                seen_sentinels.add(value)
                continue
            if isinstance(value, observe):
                if value in seen_decorated:
                    value = value.clone()
                seen_decorated.add(value)
                decorated.append(value)
                value.mangled = key
                value = value.func
                dct[key] = value
                # Coninue processing the unwrapped function
            if isinstance(value, FunctionType):
                funcs.append(value)
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
        for member in members.itervalues():
            if member.index in occupied:
                conflicts.append(member)
            else:
                occupied.add(member.index)

        resolved_index = len(occupied)
        for member in conflicts:
            clone = member.clone()
            clone.set_index(resolved_index)
            owned_members.add(clone)
            members[clone.name] = clone
            setattr(cls, clone.name, clone)
            resolved_index += 1

        # Walk the dict a second time to collect the class members. This
        # assigns the name and the index to the member. If a member is
        # overriding an existing member, the memory index of the old
        # member is reused and any static observers are copied over.
        for key, value in dct.iteritems():
            if isinstance(value, Member):
                if value in owned_members:
                    value = value.clone()
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
                member.set_default_value_mode(DefaultValue.ObjectMethod, mangled)

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
                member.set_post_validate_mode(PostValidate.ObjectMethod_OldNew, mangled)

        # _post_getattr_* methods
        n = len(POST_GETATTR_PREFIX)
        for mangled in post_getattrs:
            target = mangled[n:]
            if target in members:
                member = clone_if_needed(members[target])
                member.set_post_getattr_mode(PostGetAttr.ObjectMethod_Value, mangled)

        # _post_setattr_* methods
        n = len(POST_SETATTR_PREFIX)
        for mangled in post_setattrs:
            target = mangled[n:]
            if target in members:
                member = clone_if_needed(members[target])
                member.set_post_setattr_mode(PostSetAttr.ObjectMethod_OldNew, mangled)

        # _observe_* methods
        n = len(OBSERVE_PREFIX)
        for mangled in observes:
            target = mangled[n:]
            if target in members:
                member = clone_if_needed(members[target])
                member.add_static_observer(mangled)

        # @observe decorated methods
        for ob in decorated:
            if isinstance(ob.name, basestring):
                if ob.name in members:
                    member = clone_if_needed(members[ob.name])
                    member.add_static_observer(ob.mangled)
            else:
                for name in ob.name:
                    if name in members:
                        member = clone_if_needed(members[name])
                        member.add_static_observer(ob.mangled)

        # Put a reference to the members dict on the class. This is used
        # by CAtom to query for the members and member count as needed.
        cls.__atom_members__ = members

        return cls


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
