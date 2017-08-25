#------------------------------------------------------------------------------
# Copyright (c) 2013-2017, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
from .catom import Member, DefaultValue, Validate, SetAttr, DelAttr

from future.builtins import int


class Value(Member):
    """ A member class which supports value initialization.

    A plain `Value` provides support for default values and factories,
    but does not perform any type checking or validation. It serves as
    a useful base class for scalar members and can be used for cases
    where type checking is not needed (like private attributes).

    """
    __slots__ = ()

    def __init__(self, default=None, factory=None):
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


class ReadOnly(Value):
    """ A value which can be assigned once and is then read-only.

    """
    __slots__ = ()

    def __init__(self, default=None, factory=None):
        super(ReadOnly, self).__init__(default, factory)
        self.set_setattr_mode(SetAttr.ReadOnly, None)
        self.set_delattr_mode(DelAttr.ReadOnly, None)


class Constant(Value):
    """ A value which cannot be changed from its default.

    """
    __slots__ = ()

    def __init__(self, default=None, factory=None):
        super(Constant, self).__init__(default, factory)
        self.set_setattr_mode(SetAttr.Constant, None)
        self.set_delattr_mode(DelAttr.Constant, None)


class Callable(Value):
    """ A value which is callable.

    """
    __slots__ = ()

    def __init__(self, default=None, factory=None):
        super(Callable, self).__init__(default, factory)
        self.set_validate_mode(Validate.Callable, None)


class Bool(Value):
    """ A value of type `bool`.

    """
    __slots__ = ()

    def __init__(self, default=False, factory=None):
        super(Bool, self).__init__(default, factory)
        self.set_validate_mode(Validate.Bool, None)


class Int(Value):
    """ A value of type `int`.

    By default, ints are strictly typed.  Pass strict=False to the
    constructor to enable int casting for longs and floats.

    """
    __slots__ = ()

    def __init__(self, default=0, factory=None, strict=True):
        super(Int, self).__init__(default, factory)
        if strict:
            self.set_validate_mode(Validate.Int, None)
        else:
            self.set_validate_mode(Validate.IntPromote, None)


class Long(Value):
    """ A value of type `long`.

    By default, ints are promoted to longs. Pass strict=True to the
    constructor to enable strict long checking.

    """
    __slots__ = ()

    def __init__(self, default=int(0), factory=None, strict=False):
        super(Long, self).__init__(default, factory)
        if strict:
            self.set_validate_mode(Validate.Long, None)
        else:
            self.set_validate_mode(Validate.LongPromote, None)


class FloatRange(Value):
    """ A float value clipped to a range.

    """
    __slots__ = ()

    def __init__(self, low=None, high=None, value=None):
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
        self.set_validate_mode(Validate.FloatRange, (low, high))


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


class Bytes(Value):
    """ A value of type `bytes`.

    By default, unicode strings will be promoted to byte strings. Pass
    strict=True to the constructor to enable strict byte sting checking.

    """
    __slots__ = ()

    def __init__(self, default=b'', factory=None, strict=False):
        super(Bytes, self).__init__(default, factory)
        if strict:
            self.set_validate_mode(Validate.Bytes, None)
        else:
            self.set_validate_mode(Validate.BytesPromote, None)


class Str(Value):
    """A value of type `str`.

    Under Python 2 this is a byte string and behaves as Bytes with respect to
    promotion, under Python 3 it is a unicode string and behaves as Unicode
    with respect to promotion.

    The use of this member is discouraged in Python 2/3 compatible codebase
    as Bytes and Unicode provide a more homogeneous behavior.

    """
    def __init__(self, default='', factory=None, strict=False):
        super(Str, self).__init__(default, factory)
        if strict:
            self.set_validate_mode(Validate.String, None)
        else:
            self.set_validate_mode(Validate.StringPromote, None)


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
