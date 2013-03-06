#------------------------------------------------------------------------------
# Copyright (c) 2013, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
from .catom import Member, MemberChange, null


# XXX move this class to C++
class CachedProperty(Member):

    __slots__ = 'member'

    @staticmethod
    def reset(owner, name):
        """ Reset a cached property for the given owner.

        Parameters
        ----------
        owner : Atom
            The atom object which owns the property.

        name : str
            The name of the cached property which should be reset.

        """
        prop = owner.lookup_member(name)
        if not isinstance(prop, CachedProperty):
            msg = "'%s' is not a cached property on the '%s' object."
            raise TypeError(msg % (name, type(owner).__name__))
        prop._reset(owner)

    def __init__(self, member=None):
        if member is not None:
            assert isinstance(member, Member), 'member must be a Member'
        super(CachedProperty, self).__init__()
        self.member = member

    def _get(self, owner, name):
        getter = getattr(owner, "_get_" + name, None)
        if getter is None:
            msg = "The '%s' class does define a getter for the '%s' "
            msg += "cached property."
            raise TypeError(msg % (type(owner).__name__, name))
        return getter()

    def _set(self, owner, name, value):
        setter = getattr(owner, "_set_" + name, None)
        if setter is None:
            msg = "The cached property '%s' on the '%s' object is read-only"
            raise TypeError(msg % (name, type(owner).__name__))
        setter(value)

    def set_member_index(self, index):
        super(CachedProperty, self).set_member_index(index)
        if self.member is not None:
            self.member.set_member_index(index)

    def set_member_name(self, name):
        super(CachedProperty, self).set_member_name(name)
        if self.member is not None:
            self.member.set_member_name(name)

    def clone(self):
        clone = super(CachedProperty, self).clone()
        if self.member is not None:
            clone.member = self.member.clone()
        else:
            clone.member = None
        return clone

    def __get__(self, owner, cls):
        if owner is None:
            return self
        value = self.get_value(owner)
        if value is not null:
            return value
        value = self._get(owner, self.name)
        if self.member is not None:
            value = self.member.do_validate(owner, null, value)
        self.set_value(owner, value)
        return value

    def __set__(self, owner, value):
        name = self.name
        if self.member is not None:
            old = self.get_value(owner)
            value = self.member.do_validate(owner, old, value)
        self._set(owner, name, value)

    def _reset(self, owner):
        name = self.name
        if owner.has_observers(name):
            old = self.get_value(owner)
            new = self._get(owner, name)
            try:
                changed = old != new
            except (TypeError, ValueError):
                changed = True
            if changed:
                self.set_value(owner, new)
                change = MemberChange(owner, name, old, new)
                owner.notify_observers(name, change)
        else:
            self.set_value(owner, null)

