# --------------------------------------------------------------------------------------
# Copyright (c) 2013-2023, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
from .catom import DefaultValue, DelAttr, GetState, Member, SetAttr, Validate
from .typing_utils import extract_types


class Value(Member):
    """A member class which supports value initialization.

    A plain `Value` provides support for default values and factories,
    but does not perform any type checking or validation. It serves as
    a useful base class for scalar members and can be used for cases
    where type checking is not needed (like private attributes).

    """

    __slots__ = ()

    def __init__(self, default=None, *, factory=None):
        """Initialize a Value.

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


class ReadOnly(Value):
    """A value which can be assigned once and is then read-only."""

    __slots__ = ()

    def __init__(self, kind=None, *, default=None, factory=None):
        super(ReadOnly, self).__init__(default, factory=factory)
        self.set_setattr_mode(SetAttr.ReadOnly, None)
        self.set_delattr_mode(DelAttr.ReadOnly, None)
        self.set_getstate_mode(GetState.IncludeNonDefault, None)
        if kind:
            self.set_validate_mode(Validate.Instance, extract_types(kind))


class Constant(Value):
    """A value which cannot be changed from its default."""

    __slots__ = ()

    def __init__(self, default=None, *, factory=None, kind=None):
        super(Constant, self).__init__(default, factory=factory)
        self.set_setattr_mode(SetAttr.Constant, None)
        self.set_delattr_mode(DelAttr.Constant, None)
        self.set_getstate_mode(GetState.Exclude, None)
        if kind:
            self.set_validate_mode(Validate.Instance, extract_types(kind))


class Callable(Value):
    """A value which is callable."""

    __slots__ = ()

    def __init__(self, default=None, *, factory=None):
        super(Callable, self).__init__(default, factory=factory)
        self.set_validate_mode(Validate.Callable, None)


class Bool(Value):
    """A value of type `bool`."""

    __slots__ = ()

    def __init__(self, default=False, *, factory=None):
        super(Bool, self).__init__(default, factory=factory)
        self.set_validate_mode(Validate.Bool, None)


class Int(Value):
    """A value of type `int`.

    By default, ints are strictly typed.  Pass strict=False to the
    constructor to enable int casting for longs and floats.

    """

    __slots__ = ()

    def __init__(self, default=0, *, factory=None, strict=True):
        super(Int, self).__init__(default, factory=factory)
        if strict:
            self.set_validate_mode(Validate.Int, None)
        else:
            self.set_validate_mode(Validate.IntPromote, None)


class FloatRange(Value):
    """A float value clipped to a range.

    By default, ints and longs will be promoted to floats. Pass
    strict=True to the constructor to enable strict float checking.

    """

    __slots__ = ()

    def __init__(self, low=None, high=None, value=None, *, strict=False):
        if low is not None and high is not None and low > high:
            low, high = high, low
        default = 0.0
        if value is not None:
            default = value
        elif low is not None:
            default = low
        elif high is not None:
            default = high
        super(FloatRange, self).__init__(default)
        if strict:
            self.set_validate_mode(Validate.FloatRange, (low, high))
        else:
            self.set_validate_mode(Validate.FloatRangePromote, (low, high))


class Range(Value):
    """An integer value clipped to a range."""

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
    """A value of type `float`.

    By default, ints and longs will be promoted to floats. Pass
    strict=True to the constructor to enable strict float checking.

    """

    __slots__ = ()

    def __init__(self, default=0.0, *, factory=None, strict=False):
        super(Float, self).__init__(default, factory=factory)
        if strict:
            self.set_validate_mode(Validate.Float, None)
        else:
            self.set_validate_mode(Validate.FloatPromote, None)


class Bytes(Value):
    """A value of type `bytes`.

    By default, strings will NOT be promoted to bytes. Pass strict=False to the
    constructor to enable loose byte checking.

    """

    __slots__ = ()

    def __init__(self, default=b"", *, factory=None, strict=True):
        super(Bytes, self).__init__(default, factory=factory)
        if strict:
            self.set_validate_mode(Validate.Bytes, None)
        else:
            self.set_validate_mode(Validate.BytesPromote, None)


class Str(Value):
    """A value of type `str`.

    By default, bytes will NOT be promoted to strings. Pass strict=False to the
    constructor to enable loose string checking.

    """

    def __init__(self, default="", *, factory=None, strict=True):
        super(Str, self).__init__(default, factory=factory)
        if strict:
            self.set_validate_mode(Validate.Str, None)
        else:
            self.set_validate_mode(Validate.StrPromote, None)
