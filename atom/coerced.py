# --------------------------------------------------------------------------------------
# Copyright (c) 2013-2023, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
from .catom import DefaultValue, Member, Validate
from .typing_utils import extract_types, is_optional


class Coerced(Member):
    """A member which will coerce a value to a given instance type.

    Unlike Typed or Instance, a Coerced value is not intended to be
    set to None.

    """

    __slots__ = ()

    def __init__(self, kind, args=None, kwargs=None, *, factory=None, coercer=None):
        """Initialize a Coerced.

        Parameters
        ----------
        kind : type or tuple of types
            The allowable types for the value.

        args : tuple, optional
            If 'factory' is None, then 'kind' is a callable type and
            these arguments will be passed to the constructor to create
            the default value.

        kwargs : dict, optional
            If 'factory' is None, then 'kind' is a callable type and
            these keywords will be passed to the constructor to create
            the default value.

        factory : callable, optional
            An optional callable which takes no arguments and returns
            the default value for the member. If this is not provided
            then 'args' and 'kwargs' should be provided, as 'kind' will
            be used to generate the default value.

        coercer : callable, optional
            An optional callable which takes the value and returns the
            coerced value. If this is not given, then 'kind' must be a
            callable type which will be called with the value to coerce
            the value to the appropriate type.

        """
        origin = kind
        kind = extract_types(kind)
        opt, temp = is_optional(kind)

        if factory is not None:
            self.set_default_value_mode(DefaultValue.CallObject, factory)
        else:
            args = args or ()
            kwargs = kwargs or {}
            if opt:

                def factory():
                    return None
            else:

                def factory():
                    return kind[0](*args, **kwargs)

            self.set_default_value_mode(DefaultValue.CallObject, factory)

        if not coercer and (isinstance(origin, tuple) or len(temp) > 1):
            raise ValueError(f"No coercer was provided but {origin} is not callable.")
        self.set_validate_mode(Validate.Coerced, (kind, coercer or temp[0]))
