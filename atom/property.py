# --------------------------------------------------------------------------------------
# Copyright (c) 2013-2023, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
from .catom import DelAttr, GetAttr, GetState, Member, SetAttr, reset_property


class Property(Member):
    """A Member which behaves similar to a Python property."""

    __slots__ = ()

    def __init__(self, fget=None, fset=None, fdel=None, cached=False):
        """Initialize a Property member.

        Parameters
        ----------
        fget : callable or None, optional
            The callable invoked to get the property value. It must
            accept a single argument which is the owner object. If not
            provided, the property cannot be read. The default is None.

        fset : callable or None, optional
            The callable invoked to set the property value. It must
            accept two arguments: the owner object and property value.
            If not provided, the property cannot be set. The default
            is None.

        fdel : callable or None, optional
            The callable invoked to delete the property value. It must
            accept a single argument which is the owner object. If not
            provided, the property cannot be deleted. The default is
            None.

        cached : bool, optional
            Whether or not the property caches the computed value. A
            cached property will only evaluate 'fget' once until the
            'reset' method of the property is invoked. The default is
            False.

        """
        gm = GetAttr.CachedProperty if cached else GetAttr.Property
        self.set_getattr_mode(gm, fget)
        if cached and fset is not None:
            raise ValueError(
                "Cached property are read-only, but a setter was " "specified."
            )
        self.set_setattr_mode(SetAttr.Property, fset)
        self.set_delattr_mode(DelAttr.Property, fdel)
        self.set_getstate_mode(GetState.Property, None)

    @property
    def fget(self):
        """Get the getter function for the property.

        This will not find a specially named _get_* function.

        """
        return self.getattr_mode[1]

    @property
    def fset(self):
        """Get the setter function for the property.

        This will not find a specially named _set_* function.

        """
        return self.setattr_mode[1]

    @property
    def fdel(self):
        """Get the deleter function for the property.

        This will not find a specially named _del_* function.

        """
        return self.delattr_mode[1]

    @property
    def cached(self):
        """Test whether or not this is a cached property."""
        return self.getattr_mode[0] == GetAttr.CachedProperty

    def getter(self, func):
        """Use the given function as the property getter.

        This method is intended to be used as a decorator. The original
        function will still be callable.

        """
        mode, ignored = self.getattr_mode
        self.set_getattr_mode(mode, func)
        return func

    def setter(self, func):
        """Use the given function as the property setter.

        This method is intended to be used as a decorator. The original
        function will still be callable.

        """
        if self.cached:
            raise ValueError(
                "Cached property are read-only, but a setter was " "specified."
            )
        self.set_setattr_mode(SetAttr.Property, func)
        return func

    def deleter(self, func):
        """Use the given function as the property deleter.

        This method is intended to be used as a decorator. The original
        function will still be callable.

        """
        self.set_delattr_mode(DelAttr.Property, func)
        return func

    def reset(self, owner):
        """Reset the value of the property.

        The old property value will be cleared and the notifiers will
        be run if the new value is different from the old value. If
        the property is not cached, notifiers will be unconditionally
        run using None as the old value.

        """
        reset_property(self, owner)


def cached_property(fget):
    """A decorator which converts a function into a cached Property.

    Parameters
    ----------
    fget : callable
        The callable invoked to get the property value. It must accept
        a single argument which is the owner object.

    """
    return Property(fget, cached=True)
