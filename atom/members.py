#------------------------------------------------------------------------------
# Copyright (c) 2014, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
from .catom import CMember, CValidator


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


def _error_handler(allowed):
    """ Create an error handler for validating scalar values.

    Parameters
    ----------
    allowed : str
        The phrase describing the allowed value type(s).

    Returns
    -------
    result : callable
        A callable which accepts four arguments and raises a TypeError
        with an appropriate message indicating the validation failure.

    """
    def error_handler(validator, atom, name, value):
        msg = "The '%s' member of %s instance must be %s, "
        msg += "but a value of %s was specified."
        type_name = _add_article(type(atom).__name__)
        repr_value = '%r %r' % (value, type(value))
        raise TypeError(msg % (name, type_name, allowed, repr_value))
    return error_handler


def _range_error(validator, atom, name, value):
    """ A validation error handler for Range members.

    """
    low, high, kind = validator.context
    inst_repr = _instance_repr(kind)
    allowed = "%s in the range %r to %r inclusive" % (inst_repr, low, high)
    _error_handler(allowed)(validator, atom, name, value)


class Member(CMember):
    """ An empty subclass of the C-level member class.

    This class serves as the base of all user facing member classes. It
    allows member instances to carry an instance dict for storing user
    defined metadata.

    """
    pass


class Value(Member):
    """ A member class which supports default value initialization.

    A plain `Value` provides support for default values and factories,
    but does not perform any type checking or validation. It serves as
    a useful base class for scalar members and can be used for cases
    where type checking is not needed (such as private attributes).

    """
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
            self.default_handler = lambda atom, name: factory()
        elif default is not None:
            self.default_handler = lambda atom, name: default


class Bool(Value):
    """ A Value member which only accepts boolean values.

    """
    #: The C validator instance shared by all Bool members.
    _c_validator = CValidator(
        CValidator.Bool, False, _error_handler("a boolean value")
    )

    #: The C Validator which casts via bool(value).
    _c_cast_validator = CValidator(
        CValidator.Bool, True, _error_handler("a truth-like value")
    )

    def __init__(self, default=None, factory=None, cast=False, **metadata):
        """ Initialize a Bool member.

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

        cast : boolean, optional
            Whether to cast assigned values to booleans. This is
            equivalent to calling bool(value) on the assigned value.

        **metadata
            Additional metadata to apply to the member.

        """
        super(Bool, self).__init__(default, factory, **metadata)
        if not cast:
            self.validate_handler = Bool._c_validator
        else:
            self.validate_handler = Bool._c_cast_validator


class Int(Value):
    """ A value member which only accepts integer values.

    On Python 2, int and longs are accepted. Python 3 only has one
    integer type.

    """
    #: The C validator instance shared by all Int members.
    _c_validator = CValidator(
        CValidator.Int, False, _error_handler("an integer value")
    )

    #: The C Validator which casts via int(value).
    _c_cast_validator = CValidator(
        CValidator.Int, True, _error_handler("an int-like value")
    )

    def __init__(self, default=None, factory=None, cast=False, **metadata):
        """ Initialize an Int member.

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

        cast : boolean, optional
            Whether to cast assigned values to integers. This is
            equivalent to calling int(value) on the assigned value.

        **metadata
            Additional metadata to apply to the member.

        """
        super(Int, self).__init__(default, factory, **metadata)
        if not cast:
            self.validate_handler = Int._c_validator
        else:
            self.validate_handler = Int._c_cast_validator


class Float(Value):
    """ A value member which only accepts floating point values.

    """
    #: The C validator instance shared by all Float members.
    _c_validator = CValidator(
        CValidator.Float, False, _error_handler("a floating point value")
    )

    #: The C Validator which casts via float(value).
    _c_cast_validator = CValidator(
        CValidator.Float, True, _error_handler("a float-like value")
    )

    def __init__(self, default=None, factory=None, cast=False, **metadata):
        """ Initialize a Float member.

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

        cast : boolean, optional
            Whether to cast assigned values to floats. This is
            equivalent to calling float(value) on the assigned value.

        **metadata
            Additional metadata to apply to the member.

        """
        super(Float, self).__init__(default, factory, **metadata)
        if not cast:
            self.validate_handler = Float._c_validator
        else:
            self.validate_handler = Float._c_cast_validator


class Bytes(Value):
    """ A value member which only accepts byte strings.

    On Py2k this is the 'str' type. On Py3k this is the 'bytes' type.

    """
    #: The C validator instance shared by all Bytes members.
    _c_validator = CValidator(
        CValidator.Bytes, None, _error_handler("a byte string")
    )

    def __init__(self, default=b'', factory=None, **metadata):
        """ Initialize a Bytes member.

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
        super(Bytes, self).__init__(default, factory, **metadata)
        self.validate_handler = Bytes._c_validator


class Str(Value):
    """ A value member which only accepts strings.

    On Py2k this is the 'str' type. On Py3k this is the 'str' type.

    """
    #: The C validator instance shared by all Str members.
    _c_validator = CValidator(
        CValidator.Str, None, _error_handler("a string")
    )

    def __init__(self, default='', factory=None, **metadata):
        """ Initialize a Str member.

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
        super(Str, self).__init__(default, factory, **metadata)
        self.validate_handler = Str._c_validator


class Unicode(Value):
    """ A value member which only accepts strings.

    On Py2k this is the 'unicode' type. On Py3k this is the 'str' type.

    """
    #: The C validator instance shared by all Unicode members.
    _c_validator = CValidator(
        CValidator.Unicode, None, _error_handler("a unicode string")
    )

    def __init__(self, default=u'', factory=None, **metadata):
        """ Initialize a Unicode member.

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
        super(Unicode, self).__init__(default, factory, **metadata)
        self.validate_handler = Unicode._c_validator


class Callable(Value):
    """ A Value member which only accepts callable values.

    """
    #: The C validator instance shared by all Callable members.
    _c_validator = CValidator(
        CValidator.Callable, None, _error_handler("a callable value or None")
    )

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
        self.validate_handler = Callable._c_validator


class Range(Value):
    """ A member which accepts values within a given range.

    """
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
        default = None
        if value is not None:
            default = value
        elif low is not None:
            default = low
        elif high is not None:
            default = high
        super(Range, self).__init__(default, None, **metadata)
        context = (low, high, kind)
        validator = CValidator(CValidator.Range, context, _range_error)
        self.validate_handler = validator


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
