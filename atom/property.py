#------------------------------------------------------------------------------
# Copyright (c) 2013, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
from .catom import Member, GetAttr, SetAttr


class Property(Member):
    """ A Member which behaves similar to a Python property.

    """
    # XXX move these behaviors down to C++.
    __slots__ = ('fget', 'fset', 'cached')

    def __init__(self, fget=None, fset=None, cached=False):
        self.fget = fget
        self.fset = fset
        self.cached = cached
        if cached:
            method = "_cached_getter"
            self.set_getattr_mode(GetAttr.MemberMethod_Object, method)
        elif fget is not None:
            self.set_getattr_mode(GetAttr.CallObject_Object, fget)
        else:
            method = "_lookup_getter"
            self.set_getattr_mode(GetAttr.MemberMethod_Object, method)
        if fset is not None:
            self.set_setattr_mode(SetAttr.CallObject_ObjectValue, fset)
        else:
            method = "_lookup_setter"
            self.set_setattr_mode(SetAttr.MemberMethod_ObjectValue, method)

    #--------------------------------------------------------------------------
    # Private API
    #--------------------------------------------------------------------------
    def _cached_getter(self, owner):
        value = self.get_slot(owner)
        if value is not None: # FIXME None might be the cached value
            return value
        value = self._lookup_getter(owner)
        self.set_slot(owner, value)
        return value

    def _lookup_getter(self, owner):
        if self.fget is not None:
            return self.fget(owner)
        getter = getattr(owner, '_get_' + self.name, None)
        if getter is not None:
            return getter()
        msg = "the '%s' Property on the '%s' object is write only"
        raise TypeError(msg % (self.name, type(owner).__name__))

    def _lookup_setter(self, owner, value):
        if self.fset is not None:
            self.fset(owner, value)
        setter = getattr(owner, '_set_' + self.name, None)
        if setter is not None:
            return setter(value)
        msg = "the '%s' Property on the '%s' object is read only"
        raise TypeError(msg % (self.name, type(owner).__name__))

    #--------------------------------------------------------------------------
    # Public API
    #--------------------------------------------------------------------------
    def getter(self, func):
        """ Use the given function as the property getter.

        This method is intented to be used as a decorator.

        """
        self.fget = func
        if not self.cached:
            self.set_getattr_mode(GetAttr.CallObject_Object, func)
        return self

    def setter(self, func):
        """ Use the given function as the property getter.

        This method is intented to be used as a decorator.

        """
        self.fset = func
        self.set_setattr_mode(SetAttr.CallObject_ObjectValue, func)
        return self

    def clone(self):
        """ Create a clone of the property.

        """
        clone = super(Property, self).clone()
        clone.fget = self.fget
        clone.fset = self.fset
        clone.cached = self.cached
        return clone

    def reset(self, owner):
        """ Reset the value of the property.

        If the property is cached, the old value will be cleared and
        the notifiers (if any) will be run. If the property is not
        cached, then the notifications will be unconditionally run
        using the None as the old value.

        """
        if self.cached:
            oldvalue = self.get_slot(owner)
            self.del_slot(owner)
            if self.has_observers() or owner.has_observers(self.name):
                newvalue = self.do_getattr(owner)
                if oldvalue != newvalue:
                    change = {
                        'type': 'property',
                        'name': self.name,
                        'object': owner,
                        'oldvalue': oldvalue,
                        'value': newvalue,
                    }
                    self.notify(owner, change)       # static observers
                    owner.notify(self.name, change)  # dynamic observers
        elif self.has_observers() or owner.has_observers(self.name):
            change = {
                'type': 'property',
                'name': self.name,
                'object': owner,
                'oldvalue': None,
                'value': self.do_getattr(owner),
            }
            self.notify(owner, change)       # static observers
            owner.notify(self.name, change)  # dynamic observers


def cached_property(func):
    """ A decorator which converts a function into a cached Property.

    """
    return Property(fget=func, cached=True)
