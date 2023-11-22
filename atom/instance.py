# --------------------------------------------------------------------------------------
# Copyright (c) 2013-2023, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
from .catom import DefaultValue, GetState, Member, Validate
from .typing_utils import extract_types, is_optional


class Instance(Member):
    """A value which allows objects of a given type or types.

    Values will be tested using the `PyObject_IsInstance` C API call.
    This call is equivalent to `isinstance(value, kind)` and all the
    same rules apply.

    If optional is True, the value of an Instance may be set to None,
    otherwise None is not considered as a valid value. By default, optional will
    be considered False if a default value is provided and True otherwise.

    """

    __slots__ = ()

    def __init__(self, kind, args=None, kwargs=None, *, factory=None, optional=None):
        """Initialize an Instance.

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

        optional : bool | None, optional
            Boolean indicating if None is a valid value for the member.
            By default, the value is inferred to be True if no args or factory
            is provided.

        """
        opt, kind = is_optional(extract_types(kind))
        # Since we fast track None it is relevant to identify it early.
        if opt and optional is False:
            raise ValueError(
                "The type passed to Instance is declared optional but optional was "
                "explicitly set to False"
            )
        # If opt is False we preserve optional as None for backward compatibility.
        optional = optional if optional is not None else (opt or None)

        if factory is not None:
            self.set_default_value_mode(DefaultValue.CallObject, factory)
        elif args is not None or kwargs is not None:
            args = args or ()
            kwargs = kwargs or {}

            def factory():
                return kind[0](*args, **kwargs)

            self.set_default_value_mode(DefaultValue.CallObject, factory)
        elif optional is False:
            self.set_default_value_mode(DefaultValue.NonOptional, None)

        optional = (
            optional
            if optional is not None
            else (factory is None and args is None and kwargs is None)
        )
        if optional:
            self.set_validate_mode(Validate.OptionalInstance, kind)
        else:
            self.set_validate_mode(Validate.Instance, kind)
            # Allow to create a pickle with an unset typed value
            self.set_getstate_mode(GetState.IncludeNonDefault, None)


class ForwardInstance(Instance):
    """An Instance which delays resolving the type definition.

    The first time the value is accessed or modified, the type will
    be resolved and the forward instance will behave identically to
    a normal instance.

    """

    __slots__ = ("resolve", "args", "kwargs", "optional")

    def __init__(self, resolve, args=None, kwargs=None, *, factory=None, optional=None):
        """Initialize a ForwardInstance.

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

        optional : bool | None, optional
            Boolean indicating if None is a valid value for the member.
            By default, the value is inferred to be True if no args or factory
            is provided.

        """
        self.resolve = resolve
        self.args = args
        self.kwargs = kwargs
        if factory is not None:
            self.set_default_value_mode(DefaultValue.CallObject, factory)
        elif args is not None or kwargs is not None:
            mode = DefaultValue.MemberMethod_Object
            self.set_default_value_mode(mode, "default")
        elif optional is False:
            self.set_default_value_mode(DefaultValue.NonOptional, None)

        self.optional = (
            optional
            if optional is not None
            else (factory is None and args is None and kwargs is None)
        )
        if not self.optional:
            # Allow to create a pickle with an unset typed value
            self.set_getstate_mode(GetState.IncludeNonDefault, None)

        self.set_validate_mode(Validate.MemberMethod_ObjectOldNew, "validate")

    def default(self, owner):
        """Called to retrieve the default value.

        This is called the first time the default value is retrieved
        for the member. It resolves the type and updates the internal
        default handler to behave like a normal Instance member.

        """
        kind = self.resolve()
        args = self.args or ()
        kwargs = self.kwargs or {}

        def factory():
            return kind(*args, **kwargs)

        self.set_default_value_mode(DefaultValue.CallObject, factory)
        return kind(*args, **kwargs)

    def validate(self, owner, old, new):
        """Called to validate the value.

        This is called the first time a value is validated for the
        member. It resolves the type and updates the internal validate
        handler to behave like a normal Instance member.

        """
        kind = extract_types(self.resolve())
        if self.optional:
            self.set_validate_mode(Validate.OptionalInstance, kind)
        else:
            self.set_validate_mode(Validate.Instance, kind)
        return self.do_validate(owner, old, new)

    def clone(self):
        """Create a clone of the ForwardInstance object."""
        clone = super(ForwardInstance, self).clone()
        clone.resolve = self.resolve
        clone.args = self.args
        clone.kwargs = self.kwargs
        clone.optional = self.optional
        return clone
