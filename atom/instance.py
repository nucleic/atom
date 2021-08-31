#------------------------------------------------------------------------------
# Copyright (c) 2013-2021, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
#------------------------------------------------------------------------------
from .catom import Member, DefaultValue, Validate


class Instance(Member):
    """ A value which allows objects of a given type or types.

    Values will be tested using the `PyObject_IsInstance` C API call.
    This call is equivalent to `isinstance(value, kind)` and all the
    same rules apply.

    If optional is True, the value of an Instance may be set to None,
    otherwise None is not considered as a valid value.

    """
    __slots__ = ()

    def __init__(self, kind, args=None, kwargs=None, *, factory=None, optional=True):
        """ Initialize an Instance.

        Parameters
        ----------
        kind : type or tuple of types
            The allowed type or types for the instance.

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
            then the default value will be None, which will raised if
            accessed when optional is False.

        optional : bool, optional
            Boolean indicating if None is a valid value for the member.
            True by default.

        """
        if factory is not None:
            self.set_default_value_mode(DefaultValue.CallObject, factory)
        elif args is not None or kwargs is not None:
            args = args or ()
            kwargs = kwargs or {}
            factory = lambda: kind(*args, **kwargs)
            self.set_default_value_mode(DefaultValue.CallObject, factory)
        elif not optional:
            self.set_default_value_mode(DefaultValue.NonOptional, None)

        if optional:
            self.set_validate_mode(Validate.OptionalInstance, kind)
        else:
            self.set_validate_mode(Validate.Instance, kind)


class ForwardInstance(Instance):
    """ An Instance which delays resolving the type definition.

    The first time the value is accessed or modified, the type will
    be resolved and the forward instance will behave identically to
    a normal instance.

    """
    __slots__ = ('resolve', 'args', 'kwargs', 'optional')

    def __init__(self, resolve, args=None, kwargs=None, *, factory=None, optional=True):
        """ Initialize a ForwardInstance.

        resolve : callable
            A callable which takes no arguments and returns the type or
            tuple of types to use for validating the values.

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
            then the default value will be None, which will raised if
            accessed when optional is False.

        optional : bool, optional
            Boolean indicating if None is a valid value for the member.
            True by default.

        """
        self.resolve = resolve
        self.args = args
        self.kwargs = kwargs
        self.optional = optional
        if factory is not None:
            self.set_default_value_mode(DefaultValue.CallObject, factory)
        elif args is not None or kwargs is not None:
            mode = DefaultValue.MemberMethod_Object
            self.set_default_value_mode(mode, "default")
        elif not optional:
            self.set_default_value_mode(DefaultValue.NonOptional, None)

        self.set_validate_mode(Validate.MemberMethod_ObjectOldNew, "validate")

    def default(self, owner):
        """ Called to retrieve the default value.

        This is called the first time the default value is retrieved
        for the member. It resolves the type and updates the internal
        default handler to behave like a normal Instance member.

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
        handler to behave like a normal Instance member.

        """
        kind = self.resolve()
        if self.optional:
            self.set_validate_mode(Validate.OptionalInstance, kind)
        else:
            self.set_validate_mode(Validate.Instance, kind)
        return self.do_validate(owner, old, new)

    def clone(self):
        """ Create a clone of the ForwardInstance object.

        """
        clone = super(ForwardInstance, self).clone()
        clone.resolve = self.resolve
        clone.args = self.args
        clone.kwargs = self.kwargs
        return clone
