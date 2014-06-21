#------------------------------------------------------------------------------
# Copyright (c) 2014, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
from .catom import CMember, ValidationError


def _add_article(name):
    """ Prefix the given name with the proper article 'a' or 'an'.

    """
    if name[0].lower() in 'aeiou':
        return 'an ' + name
    return 'a ' + name


def _instance_repr(kind):
    """ Convert a type or tuple of types to a proper repr.

    """
    if not isinstance(kind, tuple):
        return _add_article(kind.__name__) + " value"
    if len(kind) == 1:
        return _add_article(kind[0].__name__) + " value"
    result = _add_article(kind[0].__name__)
    if len(kind) == 2:
        return result + " or " + kind[1].__name__
    for k in kind[1:-1]:
        result += ", " + k.__name__
    result += ", or " + kind[-1].__name__
    return result


def _range_error(validator, atom, name, value):
    """ A validation error handler for Range members.

    """
    #low, high, kind = validator.context
    #inst_repr = _instance_repr(kind)
    #allowed = "%s in the range %r to %r inclusive" % (inst_repr, low, high)
    #_error_handler(allowed)(validator, atom, name, value)


class Member(CMember):
    """ The public interface to the low-level CMember class.

    This class serves as the base of all user facing member classes.

    """
    __slots__ = ()

    def __init__(self, **metadata):
        """ Initialize a Member instance.

        Parameters
        ----------
        **metadata
            The optional metadata to store on the member.

        """
        if metadata:
            self.metadata = metadata

    def validation_error(self, atom, name, value):
        """ Raise a generic validation error for the given parameters.

        This method can be reimplemented by subclasses as needed to
        provide a more specific validation error and/or message.

        """
        raise ValidationError(name)

    def clone(self):
        raise NotImplementedError

    def tag(self):
        raise NotImplementedError


class Value(Member):
    """ A member class which supports default value initialization.

    A plain `Value` provides support for default values and factories,
    but does not perform any type checking or validation. It serves as
    a useful base class for scalar members and can be used for cases
    where type checking is not needed (such as private attributes).

    """
    __slots__ = ()

    #: The type info for the allowed type of the value. This is used
    #: to generate a reasonable validation error message, and can be
    #: overridden as needed by subclasses.
    type_info = 'an object'

    def __init__(self, default=None, factory=None, **metadata):
        """ Initialize a Value member.

        Parameters
        ----------
        default : object, optional
            The default value for the member. If this is provided, it
            should be an immutable value. The value will be shared
            among all instances of the Atom class.

        factory : callable, optional
            A callable object which is called with zero arguments and
            returns a default value for the member. This factory will
            take precedence over any value given by `default`.

        **metadata
            Additional metadata to apply to the member.

        """
        super(Value, self).__init__(**metadata)
        if factory is not None:
            self.set_default_mode(CMember.DefaultFactory, factory)
        elif default is not None:
            self.set_default_mode(CMember.DefaultValue, default)

    def validation_error(self, atom, name, value):
        """ Raise a scalar validation error for the given parameters.

        """
        type_name = _add_article(type(atom).__name__)
        repr_value = '%r %r' % (value, type(value))
        msg = "The '%s' member of %s instance must be %s, "
        msg += "but a value of %s was specified."
        msg %= (name, type_name, self.type_info, repr_value)
        raise ValidationError(msg)


class Bool(Value):
    """ A Value member which only accepts boolean values.

    """
    __slots__ = ()

    #: The type info for boolean values.
    type_info = 'a bool'

    def __init__(self, default=False, factory=None, strict=True, **metadata):
        """ Initialize a Bool member.

        Parameters
        ----------
        default : object, optional
            The default value for the member. If this is provided, it
            should be an immutable value. The value will be shared
            among all instances of the Atom class. The default is False.

        factory : callable, optional
            A callable object which is called with zero arguments and
            returns a default value for the member. This factory will
            take precedence over any value given by `default`.

        strict : boolean, optional
            Whether to enforce strict boolean checking (True), or to
            allow ints and floats to be casted to bools (False). The
            default is True.

        **metadata
            Additional metadata to apply to the member.

        """
        super(Bool, self).__init__(default, factory, **metadata)
        self.set_validate_mode(CMember.ValidateBool, strict)


class Int(Value):
    """ A value member which only accepts integer values.

    On Py2k, both ints and longs are accepted.

    """
    __slots__ = ()

    #: The type info for integer values.
    type_info = 'an int'

    def __init__(self, default=0, factory=None, strict=True, **metadata):
        """ Initialize an Int member.

        Parameters
        ----------
        default : object, optional
            The default value for the member. If this is provided, it
            should be an immutable value. The value will be shared
            among all instances of the Atom class. The default is 0.

        factory : callable, optional
            A callable object which is called with zero arguments and
            returns a default value for the member. This factory will
            take precedence over any value given by `default`.

        strict : boolean, optional
            Whether to enforce strict integer checking (True), or to
            allow floats to be casted to integers (False). The default
            is True.

        **metadata
            Additional metadata to apply to the member.

        """
        super(Int, self).__init__(default, factory, **metadata)
        self.set_validate_mode(CMember.ValidateInt, strict)


class Float(Value):
    """ A value member which only accepts floating point values.

    """
    __slots__ = ()

    #: The type info for floating point values.
    type_info = 'a float'

    def __init__(self, default=0.0, factory=None, strict=False, **metadata):
        """ Initialize a Float member.

        Parameters
        ----------
        default : object, optional
            The default value for the member. If this is provided, it
            should be an immutable value. The value will be shared
            among all instances of the Atom class. The default is 0.0.

        factory : callable, optional
            A callable object which is called with zero arguments and
            returns a default value for the member. This factory will
            take precedence over any value given by `default`.

        strict : boolean, optional
            Whether to enforce strict float checking (True), or to
            allow ints to be casted to floats (False). The default
            is False.

        **metadata
            Additional metadata to apply to the member.

        """
        super(Float, self).__init__(default, factory, **metadata)
        self.set_validate_mode(CMember.ValidateFloat, strict)


class Bytes(Value):
    """ A value member which only accepts byte strings.

    On Py2k this is the 'str' type. On Py3k this is the 'bytes' type.

    """
    __slots__ = ()

    #: The type info for byte string values.
    type_info = 'a byte string'

    def __init__(self, default=b'', factory=None, strict=False, **metadata):
        """ Initialize a Bytes member.

        Parameters
        ----------
        default : object, optional
            The default value for the member. If this is provided, it
            should be an immutable value. The value will be shared
            among all instances of the Atom class. The default is b''.

        factory : callable, optional
            A callable object which is called with zero arguments and
            returns a default value for the member. This factory will
            take precedence over any value given by `default`.

        strict : boolean, optional
            Whether to enforce strict byte string checking (True), or
            to allow unicode strings to be casted to UTF8 byte strings
            (False). The default is False.

        **metadata
            Additional metadata to apply to the member.

        """
        super(Bytes, self).__init__(default, factory, **metadata)
        self.set_validate_mode(CMember.ValidateBytes, strict)


class Str(Value):
    """ A value member which only accepts strings.

    On Py2k this is the 'str' type. On Py3k this is the 'str' type.

    """
    __slots__ = ()

    #: The type info for str values.
    type_info = 'a str'

    def __init__(self, default='', factory=None, strict=False, **metadata):
        """ Initialize a Str member.

        Parameters
        ----------
        default : object, optional
            The default value for the member. If this is provided, it
            should be an immutable value. The value will be shared
            among all instances of the Atom class. The default is ''.

        factory : callable, optional
            A callable object which is called with zero arguments and
            returns a default value for the member. This factory will
            take precedence over any value given by `default`.

        strict : boolean, optional
            Whether to enforce strict str checking (True), or to allow
            unicode/byte strings to be casted to UTF8/unicode strings
            on Py2k/Py3k (False). The default is False.

        **metadata
            Additional metadata to apply to the member.

        """
        super(Str, self).__init__(default, factory, **metadata)
        self.set_validate_mode(CMember.ValidateStr, strict)


class Unicode(Value):
    """ A value member which only accepts strings.

    On Py2k this is the 'unicode' type. On Py3k this is the 'str' type.

    """
    __slots__ = ()

    #: The type info for unicode values.
    type_info = 'a unicode string'

    def __init__(self, default=u'', factory=None, strict=False, **metadata):
        """ Initialize a Unicode member.

        Parameters
        ----------
        default : object, optional
            The default value for the member. If this is provided, it
            should be an immutable value. The value will be shared
            among all instances of the Atom class. The default is u''.

        factory : callable, optional
            A callable object which is called with zero arguments and
            returns a default value for the member. This factory will
            take precedence over any value given by `default`.

        strict : boolean, optional
            Whether to enforce strict unicode checking (True), or to
            allow UTF8 byte strings to be casted to usicode (False).
            The default is False.

        **metadata
            Additional metadata to apply to the member.

        """
        super(Unicode, self).__init__(default, factory, **metadata)
        self.set_validate_mode(CMember.ValidateUnicode, strict)


class Callable(Value):
    """ A Value member which only accepts callable values.

    """
    __slots__ = ()

    #: The type info for a callable value.
    type_info = 'a callable object'

    def __init__(self, default=None, factory=None, **metadata):
        """ Initialize a Callable member.

        Parameters
        ----------
        default : object, optional
            The default value for the member. If this is provided, it
            should be an immutable value. The value will be shared
            among all instances of the Atom class.

        factory : callable, optional
            A callable object which is called with zero arguments and
            returns a default value for the member. This factory will
            take precedence over any value given by `default`.

        **metadata
            Additional metadata to apply to the member.

        """
        super(Callable, self).__init__(default, factory, **metadata)
        self.set_validate_mode(CMember.ValidateCallable, None)


class Range(Value):
    """ A member which accepts values within a given range.

    """
    __slots__ = ()

    def __init__(self, low=None, high=None, value=None, kind=int, **metadata):
        """ Initialize a Range member.

        Parameters
        ----------
        low : object or None
            The lower limit of the range, inclusive. It must support
            rich comparison. A value of None indicates no lower bound.

        high : object or None
            The upper limit of the range, inclusive. It must support
            rich comparison. A value of None indicates no upper bound.

        value : object or None
            The default value of the range. If not provided, the low
            value will be used. If the low value is also not provided,
            the high value will be used.

        kind : type or tuple of types
            The type of value allowed in the range. This is equivalent
            to calling isinstance(value, kind). The default is int.

        **metadata
            Additional metadata to apply to the member.

        """
        if low is not None and high is None and low > high:
            low, high = high, low
        default = 0
        if value is not None:
            default = value
        elif low is not None:
            default = low
        elif high is not None:
            default = high
        super(Range, self).__init__(default, None, **metadata)
        self.set_validate_mode(CMember.ValidateRange, (low, high, kind))


class List(Value):
    pass


class Dict(Value):
    pass


class Instance(Value):
    pass


class Typed(Value):
    pass


class Subclass(Value):
    pass


class Enum(Value):
    pass


class Coerced(Value):
    pass
