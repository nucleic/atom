#------------------------------------------------------------------------------
# Copyright (c) 2013, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
from __future__ import (division, print_function, absolute_import)

from .catom import Member, DefaultValue, Validate


class Typed(Member):
    """ A value which allows objects of a given type or types.

    Values will be tested using the `PyObject_TypeCheck` C API call.
    This call is equivalent to `type(obj) in cls.mro()`. It is less
    flexible but faster than Instance. Use Instance when allowing
    heterogenous values and Typed when the value type is explicit.

    The value of a Typed may be set to None

    """
    __slots__ = ()

    def __init__(self, kind, args=None, kwargs=None, factory=None):
        """ Initialize an Typed.

        Parameters
        ----------
        kind : type
            The allowed type for the value.

        args : tuple, optional
            If 'factory' is None, then 'kind' is a callable type and
            these arguments will be passed to the constructor to create
            the default value.

        kwargs : dict, optional
            If 'factory' is None, then 'kind' is a callable type and
            these keywords will be passed to the constructor to create
            the default value.

        factory : callable, optional
            An optional factory to use for creating the default value.
            If this is not provided and 'args' and 'kwargs' is None,
            then the default value will be None.

        """
        if factory is not None:
            self.set_default_value_mode(DefaultValue.CallObject, factory)
        elif args is not None or kwargs is not None:
            args = args or ()
            kwargs = kwargs or {}
            factory = lambda: kind(*args, **kwargs)
            self.set_default_value_mode(DefaultValue.CallObject, factory)
        self.set_validate_mode(Validate.Typed, kind)


class ForwardTyped(Typed):
    """ A Typed which delays resolving the type definition.

    The first time the value is accessed or modified, the type will
    be resolved and the forward typed will behave identically to a
    normal typed.

    """
    __slots__ = ('resolve', 'args', 'kwargs')

    def __init__(self, resolve, args=None, kwargs=None, factory=None):
        """ Initialize a ForwardTyped.

        resolve : callable
            A callable which takes no arguments and returns the type to
            use for validating the values.

        args : tuple, optional
            If 'factory' is None, then 'resolve' will return a callable
            type and these arguments will be passed to the constructor
            to create the default value.

        kwargs : dict, optional
            If 'factory' is None, then 'resolve' will return a callable
            type and these keywords will be passed to the constructor to
            create the default value.

        factory : callable, optional
            An optional factory to use for creating the default value.
            If this is not provided and 'args' and 'kwargs' is None,
            then the default value will be None.

        """
        self.resolve = resolve
        self.args = args
        self.kwargs = kwargs
        if factory is not None:
            self.set_default_value_mode(DefaultValue.CallObject, factory)
        elif args is not None or kwargs is not None:
            mode = DefaultValue.MemberMethod_Object
            self.set_default_value_mode(mode, "default")
        self.set_validate_mode(Validate.MemberMethod_ObjectOldNew, "validate")

    def default(self, owner):
        """ Called to retrieve the default value.

        This is called the first time the default value is retrieved
        for the member. It resolves the type and updates the internal
        default handler to behave like a normal Typed member.

        """
        kind = self.resolve()
        args = self.args or ()
        kwargs = self.kwargs or {}
        factory = lambda: kind(*args, **kwargs)
        self.set_default_value_mode(DefaultValue.CallObject, factory)
        return kind(*args, **kwargs)

    def validate(self, owner, old, new):
        """ Called to validate the value.

        This is called the first time a value is validated for the
        member. It resolves the type and updates the internal validate
        handler to behave like a normal Typed member.

        """
        kind = self.resolve()
        self.set_validate_mode(Validate.Typed, kind)
        return self.do_validate(owner, old, new)

    def clone(self):
        """ Create a clone of the ForwardTyped instance.

        """
        clone = super(ForwardTyped, self).clone()
        clone.resolve = self.resolve
        clone.args = self.args
        clone.kwargs = self.kwargs
        return clone
