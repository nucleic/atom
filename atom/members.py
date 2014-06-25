#------------------------------------------------------------------------------
# Copyright (c) 2014, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
from .catom import CMember, ValidationError
from . import formatting


class Member(CMember):
    """ The public interface to the low-level CMember class.

    This class serves as the base of all user facing member classes.

    """
    __slots__ = ()

    #: The type info for the allowed type of the value. This is used
    #: to generate a reasonable validation error message, and can be
    #: overridden as needed by subclasses.
    type_info = 'an object'

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
        msg = formatting.member_message(self, atom, name, value)
        raise ValidationError(msg)

    def tag(self, **kwargs):
        """ Add metadata to the member.

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
            self.metadata = kwargs
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
            self.set_default_mode(CMember.DefaultFactory, factory)
        elif default is not None:
            self.set_default_mode(CMember.DefaultValue, default)


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


class Tuple(Member):
    """ A member which allows a tuple of optionally typed values.

    If item validation is used, then assignment will create a copy of
    the tuple before validating the items. This is required since the
    item validation may change the item values.

    """
    __slots__ = ()

    def __init__(self, item=None, default=(), **metadata):
        """ Initialize a Tuple member.

        Parameters
        ----------
        item : Member, type, or tuple of types, optional
            A member to use for validating the types of items allowed
            in the tuple. This can also be a type object or a tuple of
            types, in which case it will be wrapped with an Instance
            member. If not given, no item validation is performed.

        default : tuple, optional
            The default tuple of values.

        **metadata
            Additional metadata to apply to the member.

        """
        super(Tuple, self).__init__(**metadata)
        if item is not None and not isinstance(item, Member):
            item = Instance(item)
        self.set_default_mode(CMember.DefaultValue, default)
        self.set_validate_mode(CMember.ValidateTuple, item)

    @property
    def type_info(self):
        """ The type info for a Tuple member.

        """
        item = self.validate_mode()[1]
        if item is None:
            return 'a tuple'
        return 'a tuple of items which are ' + item.type_info


class List(Member):
    """ A member which allows a list of optionally typed values.

    Assigning to a List member creates a copy of the list. The orginal
    list will remain unmodified. This is similar to the semantics of the
    assignment operator of C++ STL container classes. Copying the list
    is required to ensure correct error reporting and internal ownership
    semantics. If traditional non-copying list behavior is desired, use
    a Typed(list) member instead.

    """
    __slots__ = ()

    def __init__(self, item=None, default=None, **metadata):
        """ Initialize a List member.

        Parameters
        ----------
        item : Member, type, or tuple of types, optional
            A member to use for validating the types of items allowed in
            the list. This can also be a type object or a tuple of types,
            in which case it will be wrapped with an Instance member. If
            this is not given, no item validation is performed.

        default : list, optional
            The default list of values. A new copy of this list will be
            created for each atom instance.

        **metadata
            Additional metadata to apply to the member.

        """
        super(List, self).__init__(**metadata)
        if item is not None and not isinstance(item, Member):
            item = Instance(item)
        self.set_default_mode(CMember.DefaultList, default)
        self.set_validate_mode(CMember.ValidateList, item)

    @property
    def type_info(self):
        """ The type info for a List member.

        """
        item = self.validate_mode()[1]
        if item is None:
            return 'a list'
        return 'a list of items which are ' + item.type_info


class Dict(Member):
    """ A member which allows a dict of optionally typed values.

    Assigning to a Dict member creates a copy of the dict. The orginal
    dict will remain unmodified. This is similar to the semantics of the
    assignment operator of C++ STL container classes. Copying the dict
    is required to ensure correct error reporting and internal ownership
    semantics. If traditional non-copying dict behavior is desired, use
    a Typed(dict) member instead.

    """
    __slots__ = ()

    def __init__(self, key=None, value=None, default=None, **metadata):
        """ Initialize a Dict member.

        Parameters
        ----------
        key : Member, type, tuple of types, or None, optional
            A member to use for validating the types of keys allowed in
            the dict. This can also be a type or a tuple of types, which
            will be wrapped with an Instance member. If this is not
            given, no key validation is performed.

        value : Member, type, tuple of types, or None, optional
            A member to use for validating the types of values allowed
            in the dict. This can also be a type or a tuple of types,
            which will be wrapped with an Instance member. If this is
            not given, no value validation is performed.

        default : dict, optional
            The default dict of items. A new copy of this dict will be
            created for each atom instance.

        **metadata
            Additional metadata to apply to the member.

        """
        super(Dict, self).__init__(**metadata)
        if key is not None and not isinstance(key, Member):
            key = Instance(key)
        if value is not None and not isinstance(value, Member):
            value = Instance(value)
        self.set_default_mode(CMember.DefaultDict, default)
        self.set_validate_mode(CMember.ValidateDict, (key, value))

    @property
    def type_info(self):
        """ The type info for a Dict member.

        """
        key, val = self.validate_mode()[1]
        if key is None and val is None:
            return 'a dict'
        key_info = key.type_info if key is not None else 'an object'
        val_info = val.type_info if val is not None else 'an object'
        msg = 'a dict with keys which are %s and with values which are %s'
        return msg % (key_info, val_info)


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
        self.set_validate_mode(CMember.ValidateTyped, kind)

    @property
    def type_info(self):
        """ The type info for a Typed member.

        """
        kind = self.validate_mode()[1]
        return formatting.add_article(kind.__name__)


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
        self.set_validate_mode(CMember.ValidateInstance, kind)

    @property
    def type_info(self):
        """ The type info for an Instance member.

        """
        kind = self.validate_mode()[1]
        return formatting.instance_repr(kind)


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
        self.set_validate_mode(CMember.ValidateSubclass, kind)

    @property
    def type_info(self):
        """ The type info for a Subclass member.

        """
        kind = self.validate_mode()[1]
        return formatting.subclass_repr(kind)


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
        self.set_validate_mode(CMember.ValidateEnum, items)

    @property
    def type_info(self):
        """ The type info for an Enum member.

        """
        items = self.validate_mode()[1]
        return 'one of %s' % list(items)

    @property
    def items(self):
        """ The items sequence for the Enum member.

        """
        return self.validate_mode()[1]

    def __call__(self, item):
        """ Create a clone of the Enum with a new default value.

        Parameters
        ----------
        item : object
            The item to use as the new Enum default.

        Returns
        -------
        result : Enum
            The cloned member with the updated default value.

        """
        clone = self.clone()
        clone.set_default_mode(CMember.DefaultValue, item)
        return clone


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
        self.set_validate_mode(CMember.ValidateRange, (low, high, kind))

    @property
    def type_info(self):
        """ The type info for a Range member.

        """
        low, high, kind = self.validate_mode()[1]
        if low is None:
            low = '-infinity'
        if high is None:
            high = 'infinity'
        name = formatting.instance_repr(kind)
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
        self.set_validate_mode(CMember.ValidateCoerced, (kind, coercer))

    @property
    def type_info(self):
        """ The type info for a Coerced member.

        """
        kind = self.validate_mode()[1][0]
        return formatting.coerced_repr(kind)
