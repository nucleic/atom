#------------------------------------------------------------------------------
# Copyright (c) 2013, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
from .catom import Member, DefaultValue, Validate, SetAttr
from .coerced import Coerced

class Value(Member):
    """ A member class which supports value initialization.

    A plain `Value` provides support for default values and factories,
    but does not perform any type checking or validation. It serves as
    a useful base class for scalar members and can be used for cases
    where type checking is not needed (like private attributes).

    """
    __slots__ = ()

    def __init__(self, default=None, factory=None, args=None, kwargs=None,
                 strict=True):
        """ Initialize a Value.

        Parameters
        ----------
        default : object, optional
            The default value for the member. If this is provided, it
            should be an immutable value. The value will will not be
            copied between owner instances.

        factory : callable, optional
            A callable object which is called with zero arguments and
            returns a default value for the member. This will override
            any value given by `default`.

        """
        if factory is not None:
            self.set_default_value_mode(DefaultValue.CallObject, factory)
        else:
            self.set_default_value_mode(DefaultValue.Static, default)

    def _set_coerced(self, kind, default=None, args=None, kwargs=None,
                     factory=None, coercer=None):
        """ Define the coercion behavior for this class

        Parameters
        ----------
        kind : type or tuple of types
            The allowable types for the value.

        default : object, optional
            The default value for the member. If this is provided, it
            should be an immutable value. The value will will not be
            copied between owner instances.

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
        if default and factory is None:
            if args is None:
                args = [default]
            else:
                args.index(0, default)
        if factory is not None:
            self.set_default_value_mode(DefaultValue.CallObject, factory)
        else:
            args = args or ()
            kwargs = kwargs or {}
            if not coercer is None:
                factory = lambda: coercer(*args, **kwargs)
            else:
                factory = lambda: kind(*args, **kwargs)
            self.set_default_value_mode(DefaultValue.CallObject, factory)
        self.set_validate_mode(Validate.Coerced, (kind, coercer or kind))


class ReadOnly(Value):
    """ A value which can be assigned once and is then read-only.

    """
    __slots__ = ()

    def __init__(self, default=None, factory=None):
        super(ReadOnly, self).__init__(default, factory)
        self.set_setattr_mode(SetAttr.ReadOnly, None)


class Constant(Value):
    """ A value which cannot be changed from its default.

    """
    __slots__ = ()

    def __init__(self, default=None, factory=None):
        super(Constant, self).__init__(default, factory)
        self.set_setattr_mode(SetAttr.Constant, None)


class Callable(Value):
    """ A value which is callable.

    """
    __slots__ = ()

    def __init__(self, default=None, factory=None):
        super(Callable, self).__init__(default, factory)
        self.set_validate_mode(Validate.Callable, None)


class Bool(Value):
    """ A value of type `bool`.

    Pass strict=False to constructor to enable coercion.
    """
    __slots__ = ()

    def __init__(self, default=False, factory=None, strict=True):
        super(Bool, self).__init__(default, factory)
        if strict:
            self.set_validate_mode(Validate.Bool, None)
        else:
            self._set_coerced(bool, factory=factory, default=default)


class Int(Value):
    """ A value of type `int`.

    """
    __slots__ = ()

    def __init__(self, default=0, factory=None):
        super(Int, self).__init__(default, factory)
        self.set_validate_mode(Validate.Int, None)


class Long(Value):
    """ A value of type `long`.

    By default, ints are promoted to longs. Pass strict=True to the
    constructor to enable strict long checking.

    """
    __slots__ = ()

    def __init__(self, default=0L, factory=None, strict=False):
        super(Long, self).__init__(default, factory)
        if strict:
            self.set_validate_mode(Validate.Long, None)
        else:
            self.set_validate_mode(Validate.LongPromote, None)


class Range(Value):
    """ An integer value clipped to a range.

    """
    __slots__ = ()

    def __init__(self, low=None, high=None, value=None):
        if low is not None and high is not None and low > high:
            low, high = high, low
        default = 0
        if value is not None:
            default = value
        elif low is not None:
            default = low
        elif high is not None:
            default = high
        super(Range, self).__init__(default)
        self.set_validate_mode(Validate.Range, (low, high))


class Float(Value):
    """ A value of type `float`.

    By default, ints and longs will be promoted to floats. Pass
    strict=True to the constructor to enable strict float checking.

    """
    __slots__ = ()

    def __init__(self, default=0.0, factory=None, strict=False):
        super(Float, self).__init__(default, factory)
        if strict:
            self.set_validate_mode(Validate.Float, None)
        else:
            self.set_validate_mode(Validate.FloatPromote, None)


class Str(Value):
    """ A value of type `str`.

    """
    __slots__ = ()

    def __init__(self, default='', factory=None):
        super(Str, self).__init__(default, factory)
        self.set_validate_mode(Validate.Str, None)


class Unicode(Value):
    """ A value of type `unicode`.

    By default, plain strings will be promoted to unicode strings. Pass
    strict=True to the constructor to enable strict unicode checking.

    """
    __slots__ = ()

    def __init__(self, default=u'', factory=None, strict=False):
        super(Unicode, self).__init__(default, factory)
        if strict:
            self.set_validate_mode(Validate.Unicode, None)
        else:
            self.set_validate_mode(Validate.UnicodePromote, None)
