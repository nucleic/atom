#------------------------------------------------------------------------------
# Copyright (c) 2013, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
from .catom import (
    Member, DEFAULT_FACTORY, VALIDATE_TYPED, VALIDATE_MEMBER_METHOD
)


class Typed(Member):
    """ A value which allows objects of a given type or types.

    Values will be tested using the `PyObject_TypeCheck` C API call.
    This call is equivalent to `type(obj) in cls.mro()`. It is less
    flexible but faster than Instance.

    A typed value may not be set to None. However, it may be set to
    null. This is chosen to more explicititly indicate the intent of
    Typed versus Instance.

    """
    __slots__ = ()

    def __init__(self, kind, factory=None):
        """ Initialize an Instance.

        Parameters
        ----------
        kind : type
            The allowed type for the instance.

        factory : callable, optional
            An optional factory to use for creating the default value.
            If this is not provided and 'args' and 'kwargs' is None,
            then the default value will be null.

        """
        if factory is not None:
            self.set_default_kind(DEFAULT_FACTORY, factory)
        self.set_validate_kind(VALIDATE_TYPED, kind)


class ForwardTyped(Typed):
    """ A Typed which delays resolving the type definition.

    The first time the value is accessed or modified, the type will
    be resolved and the forward instance will behave identically to
    a normal instance.

    """
    __slots__ = 'resolve'

    def __init__(self, resolve, factory=None):
        """ Initialize a ForwardTyped.

        resolve : callable
            A callable which takes no arguments and returns the type to
            use for validating the values.

        factory : callable, optional
            An optional factory to use for creating the default value.
            If this is not provided and 'args' and 'kwargs' is None,
            then the default value will be None.

        """
        self.resolve = resolve
        if factory is not None:
            self.set_default_kind(DEFAULT_FACTORY, factory)
        self.set_validate_kind(VALIDATE_MEMBER_METHOD, "validate")

    def validate(self, owner, old, new):
        """ Called to validate the value.

        This will resolve the type and validate the new value. It will
        then update the internal default and validate handlers to behave
        like a normal instance member.

        """
        resolve = self.validate_kind[1]
        kind = resolve()
        if type(new) not in kind.mro():
            raise TypeError('invalid type')
        self.set_validate_kind(VALIDATE_TYPED, kind)
        return new

