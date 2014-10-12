#------------------------------------------------------------------------------
# Copyright (c) 2014, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
from .catom import CMember, TypedDict, TypedList, ValidationError
from .formatting import add_article, kind_repr


class Member(CMember):
    """ The public interface to the low-level CMember class.

    This class serves as the base of all user facing member classes.

    """
    __slots__ = ()

    #: The type info for the allowed type of the value. This is used
    #: to generate a reasonable validation error message, and can be
    #: overridden as needed by subclasses.
    type_info = 'an object'

    def validation_error(self, atom, name, value):
        """ Raise a generic ValidationError for the given parameters.

        This is called by the C++ layer when value validation fails.

        This method may be reimplemented by subclasses if necessary
        in order to raise a custom ValidationError.

        Parameters
        ----------
        atom : Atom
            The atom object which owns the value.

        name : str
            The name of the attribute being validated.

        value : object
            The value which failed validation.

        Raises
        ------
        ValidationError

        """
        value_repr = '%r %r' % (value, type(value))
        atom_name = add_article(type(atom).__name__)
        args = (name, atom_name, self.type_info, value_repr)
        msg = ("The '%s' member of %s instance must be %s, "
               "but a value of %s was specified.") % args
        raise ValidationError(msg)

    def tag(self, **kwargs):
        """ Add additional metadata to the member.

        This method is a convenience to add metadata to the member
        when passing the data to the constructor is not feasible.

        Parameters
        ----------
        **kwargs
            Metadata to apply to the member.

        Returns
        -------
        result : Member
            Returns 'self' to enable this method to be conveniently
            used within the class body declaration.

        """
        metadata = self.metadata
        if metadata is None:
            self.metadata = kwargs.copy()
        else:
            metadata.update(kwargs)
        return self


class Value(Member):
    """ A member class which supports default value initialization.

    A plain `Value` provides support for default values and factories,
    but does not perform any type checking or validation. It serves as
    a useful base class for scalar members and can be used for cases
    where type checking is not needed (such as private attributes).

    """
    __slots__ = ()

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
            self.default_mode = (CMember.DefaultFactory, factory)
        elif default is not None:
            self.default_mode = (CMember.DefaultValue, default)


class Bool(Value):
    """ A value member which only accepts boolean values.

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
        self.validate_mode = (CMember.ValidateBool, strict)


class Int(Value):
    """ A value member which only accepts integer values.

    On Python 2.x, both ints and longs are accepted.

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
        self.validate_mode = (CMember.ValidateInt, strict)


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
        self.validate_mode = (CMember.ValidateFloat, strict)


class Bytes(Value):
    """ A value member which only accepts byte strings.

    On Python 2.x this is the 'str' type.
    On Python 3.x this is the 'bytes' type.

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
        self.validate_mode = (CMember.ValidateBytes, strict)


class Str(Value):
    """ A value member which only accepts strings.

    On both Python 2.x and Python 3.x this is the 'str' type.

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
        self.validate_mode = (CMember.ValidateStr, strict)


class Unicode(Value):
    """ A value member which only accepts unicode strings.

    On Python 2.x this is the 'unicode' type.
    On Python 3.x this is the 'str' type.

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
        self.validate_mode = (CMember.ValidateUnicode, strict)


class Typed(Value):
    """ A value member which allows objects of a given type.

    Typed validation is performed using a C-api call which is faster
    than the equivalent `isinstance` but only allows checking against
    a single type. A Typed member should be used over an Instance when
    multiple value types or `__instancecheck__` hooks are not required.

    The value of a Typed member may also be set to None.

    """
    __slots__ = ()

    def __init__(self, kind, args=None, kwargs=None, factory=None, **metadata):
        """ Initialize an Typed member.

        Parameters
        ----------
        kind : type
            The allowed type for the value

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

        **metadata
            Additional metadata to apply to the member.

        """
        if factory is None and (args is not None or kwargs is not None):
            args = args or ()
            kwargs = kwargs or {}
            factory = lambda: kind(*args, **kwargs)
        super(Typed, self).__init__(None, factory, **metadata)
        self.validate_mode = (CMember.ValidateTyped, kind)

    @property
    def type_info(self):
        """ The type info for a Typed member.

        """
        kind = self.validate_mode[1]
        return add_article(kind.__name__)


class Instance(Value):
    """ A value member which allows objects of a given type or types.

    Instance validation is performed using the C-api equivalent of a
    call to `isinstance(value, kind)` and all the same rules apply.

    The value of an Instance member may also be set to None.

    """
    __slots__ = ()

    def __init__(self, kind, args=None, kwargs=None, factory=None, **metadata):
        """ Initialize an Instance member.

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
            then the default value will be None.

        **metadata
            Additional metadata to apply to the member.

        """
        if factory is None and (args is not None or kwargs is not None):
            args = args or ()
            kwargs = kwargs or {}
            factory = lambda: kind(*args, **kwargs)
        super(Instance, self).__init__(None, factory, **metadata)
        self.validate_mode = (CMember.ValidateInstance, kind)

    @property
    def type_info(self):
        """ The type info for an Instance member.

        """
        kind = self.validate_mode[1]
        return 'an instance of ' + kind_repr(kind)


class Subclass(Value):
    """ A value member which allows types of a given subtype or subtypes.

    Subclass validation is performed using the C-api equivalent of a
    call to `issubclass(value, kind)` and all the same rules apply.

    The value of a Subclass member may also be set to None.

    """
    __slots__ = ()

    def __init__(self, kind, default=None, factory=None, **metadata):
        """ Initialize a Subclass member.

        Parameters
        ----------
        kind : type or tuple of types
            The allowed type or types for the subclass. This will be
            used as the default value if no default is given.

        default : type, optional
            The default value for the member.

        factory : callable, optional
            A callable object which is called with zero arguments and
            returns a default value for the member. This factory will
            take precedence over any value given by `default`.

        **metadata
            Additional metadata to apply to the member.

        """
        super(Subclass, self).__init__(default, factory, **metadata)
        self.validate_mode = (CMember.ValidateSubclass, kind)

    @property
    def type_info(self):
        """ The type info for a Subclass member.

        """
        kind = self.validate_mode[1]
        return 'a subclass of ' + kind_repr(kind)


class Enum(Value):
    """ A value member which accepts one of a sequence of items.

    """
    __slots__ = ()

    def __init__(self, items, default=None, factory=None, **metadata):
        """ Initialize an Enum member.

        Parameters
        ----------
        items : sequence
            The allowed values which can be assigned to the enum.

        default : object, optional
            The default value for the member. If this is not given,
            the first value from the items sequence will be used.

        factory : callable, optional
            A callable object which is called with zero arguments and
            returns a default value for the member. This factory will
            take precedence over any value given by `default`.

        **metadata
            Additional metadata to apply to the member.

        """
        if len(items) == 0:
            raise ValueError('an Enum sequence requires at least 1 item')
        if default is None:
            default = items[0]
        super(Enum, self).__init__(default, factory, **metadata)
        self.validate_mode = (CMember.ValidateEnum, items)

    @property
    def type_info(self):
        """ The type info for an Enum member.

        """
        items = self.validate_mode[1]
        return 'one of %s' % list(items)


class Callable(Value):
    """ A value member which only accepts callable values.

    The value of an Callable member may also be set to None.

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
        self.validate_mode = (CMember.ValidateCallable, None)


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
            the high value will be used. In the absence of any given
            value, the default will be 0.

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
        self.validate_mode = (CMember.ValidateRange, (low, high, kind))

    @property
    def type_info(self):
        """ The type info for a Range member.

        """
        low, high, kind = self.validate_mode[1]
        if low is None:
            low = '-infinity'
        if high is None:
            high = 'infinity'
        name = 'an instance of ' + kind_repr(kind)
        return '%s in the range %s to %s inclusive' % (name, low, high)


class Coerced(Value):
    """ A value member which will coerce a value to a given type.

    Unlike Typed or Instance, a Coerced member can not be set to None
    unless NoneType is indicated as an acceptable value type.

    """
    __slots__ = ()

    def __init__(self, kind, args=None, kwargs=None,
                 factory=None, coercer=None, **metadata):
        """ Initialize a Coerced member.

        Parameters
        ----------
        kind : type or tuple of types
            The allowed type or types for the coerced value.

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
            If this is not provided then 'args' and 'kwargs' should be
            provided, as 'kind' will be called to create the default.

        coercer : callable, optional
            An optional callable which takes the value and returns the
            coerced value. If this is not given, then 'kind' must be a
            callable type which will be called with the value to coerce
            the value to the appropriate type.

        **metadata
            Additional metadata to apply to the member.

        """
        if factory is None:
            args = args or ()
            kwargs = kwargs or {}
            factory = lambda: kind(*args, **kwargs)
        super(Coerced, self).__init__(None, factory, **metadata)
        if coercer is None:
            coercer = kind
        self.validate_mode = (CMember.ValidateCoerced, (kind, coercer))

    @property
    def type_info(self):
        """ The type info for a Coerced member.

        """
        kind = self.validate_mode[1][0]
        return 'coercible to ' + kind_repr(kind)


class List(Value):
    """ A member which accepts a list of a given element type.

    A List() member has copy-on-assignment semantics similar to C++
    containers. This behavior is required to ensure that in-place
    modifications to the list can be type-checked.

    If non-copying behavior (reference semantics) is required, use
    a Typed(list) member instead. Note that in-place modifications
    to such a list cannot be type-checked.

    If reference semantics AND in-place type-checking is required,
    use a Typed(TypedList, (value_type,)) member. Note that direct
    assignment of a list literal to such a member is not valid. It
    is also possible to assign a TypedList with a different value
    type to such a member.

    """
    __slots__ = ()

    def __init__(self, value_type=object,
                 default=[], factory=None, **metadata):
        """ Initialize a List member.

        Parameters
        ----------
        value_type : type or tuple of types
            The allowed type or types for the list elements.

        default : list, optional
            The default list for the member.

        factory : callable, optional
            A callable object which is called with zero arguments and
            returns a default list for the member. This factory will
            take precedence over any value given by `default`.

        **metadata
            Additional metadata to apply to the member.

        """
        super(List, self).__init__(default, factory, **metadata)
        self.validate_mode = (CMember.ValidateList, (value_type, TypedList))

    @property
    def type_info(self):
        """ The type info for a List member.

        """
        value_type = self.validate_mode[1][0]
        return 'a list of ' + kind_repr(value_type)


class Dict(Value):
    """ A member which accepts a dict of a given key and value type.

    A Dict() member has copy-on-assignment semantics similar to C++
    containers. This behavior is required to ensure that in-place
    modifications to the dict can be type-checked.

    If non-copying behavior (reference semantics) is required, use
    a Typed(dict) member instead. Note that in-place modifications
    to such a dict cannot be type-checked.

    If reference semantics AND in-place type-checking is required,
    use a Typed(TypedDict, (key_type, value_type)) member. Note that
    direct assignment of a dict literal to such a member is not valid.
    It is also possible to assign a TypedDict with a different type
    specification to such a member.

    """
    __slots__ = ()

    def __init__(self, key_type=object, value_type=object,
                 default={}, factory=None, **metadata):
        """ Initialize a Dict member.

        Parameters
        ----------
        key_type : type or tuple of types
            The allowed type or types for the dict keys.

        value_type : type or tuple of types
            The allowed type or types for the dict values.

        default : dict, optional
            The default dict for the member.

        factory : callable, optional
            A callable object which is called with zero arguments and
            returns a default dict for the member. This factory will
            take precedence over any value given by `default`.

        **metadata
            Additional metadata to apply to the member.

        """
        super(Dict, self).__init__(default, factory, **metadata)
        mode_data = (key_type, value_type, TypedDict)
        self.validate_mode = (CMember.ValidateDict, mode_data)

    @property
    def type_info(self):
        """ The type info for a Dict member.

        """
        mode_data = self.validate_mode[1]
        key_repr = kind_repr(mode_data[0])
        value_repr = kind_repr(mode_data[1])
        return 'a dict of (%s, %s)' % (key_repr, value_repr)
