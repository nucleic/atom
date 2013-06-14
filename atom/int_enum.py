#------------------------------------------------------------------------------
# Copyright (c) 2013, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
# Note: This module is imported by 'atom.catom' module from code defined in
# the 'enumtypes.cpp' file. This module must therefore not import atom.
import copy_reg


# IntEnum is not defined until the metaclass creates it.
IntEnum = None


class _IntEnumMeta(type):
    """ The metaclass for IntEnum and its subclasses.

    """
    def __new__(meta, name, bases, dct):
        if len(bases) > 1:
            raise TypeError('int enums do not support multiple inheritance')
        if IntEnum is None:
            return type.__new__(meta, name, bases, dct)
        if bases[0] is not IntEnum:
            raise TypeError('int enums cannot be extended')
        enums = {}
        reved = {}
        cls = type.__new__(meta, name, bases, dct)
        for key, value in cls.__dict__.iteritems():
            if isinstance(value, int):
                enum = int.__new__(cls, value)
                enum.__enum_name__ = key
                type.__setattr__(cls, key, enum)
                enums[key] = enum
                reved[int(enum)] = enum
        type.__setattr__(cls, '__enums__', enums)
        type.__setattr__(cls, '__reved__', reved)
        return cls

    def __call__(cls, which):
        enum = None
        if isinstance(which, int):
            enum = cls.__reved__.get(which)
        elif isinstance(which, basestring):
            enum = cls.__enums__.get(which)
        else:
            msg = "enum specifier must be an int or basestring, "
            msg += "got object of type '%s' instead"
            raise TypeError(msg % type(which).__name__)
        if enum is None:
            msg = "'%s' is not a valid %s enum specifier"
            raise ValueError(msg % (which, cls.__name__))
        return enum

    def __contains__(cls, which):
        return which in cls.__reved__ or which in cls.__enums__

    def __getitem__(cls, which):
        return cls(which)

    def __len__(cls):
        return len(cls.__enums__)

    def __iter__(cls):
        return iter(cls.__enums__.values())

    def __setattr__(cls, name, value):
        if name in cls.__enums__:
            raise TypeError("cannot change the value of an enum")
        type.__setattr__(cls, name, value)


def _invalid_op(op):
    msg = " is an invalid operation for %s"
    msg = ("'%s'" % op) + msg
    def closure(self, *args):
        raise TypeError(msg % self)
    return closure


class IntEnum(int):
    """ An integer subclass for declaring enum types.

    """
    __metaclass__ = _IntEnumMeta

    __flags_class__ = None

    def __make_flags(self, other, value):
        scls = type(self)
        ocls = type(other)
        fcls = scls.__flags_class__
        if ocls is not scls and ocls is not fcls:
            msg = "cannot combine %s with object of type '%s'"
            raise TypeError(msg % (self, ocls.__name__))
        if fcls is None:
            msg = "cannot combine %s and %s - no flags class is defined"
            raise TypeError(msg % (self, other))
        return int.__new__(fcls, value)

    def __reduce_ex__(self, proto):
        return (type(self), (int(self),))

    def __repr__(self):
        t_name = type(self).__name__
        s_name = self.__enum_name__
        return '<enum: %s.%s [value=%d]>' % (t_name, s_name, self)

    def __str__(self):
        return '%s.%s' % (type(self).__name__, self.__enum_name__)

    def __and__(self, other):
        return self.__make_flags(other, int.__and__(self, other))

    __rand__ = __and__

    def __or__(self, other):
        return self.__make_flags(other, int.__or__(self, other))

    __ror__ = __or__

    def __xor__(self, other):
        return self.__make_flags(other, int.__xor__(self, other))

    __rxor__ = __xor__

    def __invert__(self):
        return self.__make_flags(self, int.__invert__(self))

    __add__ = _invalid_op('+')
    __sub__ = _invalid_op('-')
    __mul__ = _invalid_op('*')
    __div__ = _invalid_op('/')
    __truediv__ = _invalid_op('/')
    __floordiv__ = _invalid_op('//')
    __mod__ = _invalid_op('%')
    __divmod__ = _invalid_op('divmod')
    __pow__ = _invalid_op('**')
    __lshift__ = _invalid_op('<<')
    __rshift__ = _invalid_op('>>')
    __radd__ = _invalid_op('+')
    __rsub__ = _invalid_op('-')
    __rmul__ = _invalid_op('*')
    __rdiv__ = _invalid_op('/')
    __rtruediv__ = _invalid_op('/')
    __rfloordiv__ = _invalid_op('//')
    __rmod__ = _invalid_op('%')
    __rdivmod__ = _invalid_op('divmod')
    __rpow__ = _invalid_op('**')
    __rlshift__ = _invalid_op('<<')
    __rrshift__ = _invalid_op('>>')
    __neg__ = _invalid_op('-')
    __pos__ = _invalid_op('+')
    __abs__ = _invalid_op('abs')

    @property
    def name(self):
        return self.__enum_name__


class _IntEnumFlags(int):

    __enum_class__ = None

    def __new__(cls, value=0, force=False):
        if isinstance(value, cls):
            return value
        if value == 0:
            return int.__new__(cls, value)
        ecls = cls.__enum_class__
        if ecls is None:
            msg = "cannot create %s - no enum class is defined"
            raise TypeError(msg % cls.__name__)
        return int.__new__(cls, ecls(value))

    def __make_flags(self, other, value):
        scls = type(self)
        ocls = type(other)
        ecls = scls.__enum_class__
        if ocls is not scls and ocls is not ecls:
            msg = "cannot combine %s with object of type '%s'"
            raise TypeError(msg % (self, ocls.__name__))
        if ecls is None:
            msg = "cannot combine %s and %s - no enum class is defined"
            raise TypeError(msg % (self, other))
        return int.__new__(scls, value)

    def __repr__(self):
        return '<enumflags: %s [value=%d]>' % (type(self).__name__, self)

    def __str__(self):
        return type(self).__name__

    def __and__(self, other):
        return self.__make_flags(other, int.__and__(self, other))

    __rand__ = __and__

    def __or__(self, other):
        return self.__make_flags(other, int.__or__(self, other))

    __ror__ = __or__

    def __xor__(self, other):
        return self.__make_flags(other, int.__xor__(self, other))

    __rxor__ = __xor__

    def __invert__(self):
        return self.__make_flags(self, int.__invert__(self))

    __add__ = _invalid_op('+')
    __sub__ = _invalid_op('-')
    __mul__ = _invalid_op('*')
    __div__ = _invalid_op('/')
    __truediv__ = _invalid_op('/')
    __floordiv__ = _invalid_op('//')
    __mod__ = _invalid_op('%')
    __divmod__ = _invalid_op('divmod')
    __pow__ = _invalid_op('**')
    __lshift__ = _invalid_op('<<')
    __rshift__ = _invalid_op('>>')
    __radd__ = _invalid_op('+')
    __rsub__ = _invalid_op('-')
    __rmul__ = _invalid_op('*')
    __rdiv__ = _invalid_op('/')
    __rtruediv__ = _invalid_op('/')
    __rfloordiv__ = _invalid_op('//')
    __rmod__ = _invalid_op('%')
    __rdivmod__ = _invalid_op('divmod')
    __rpow__ = _invalid_op('**')
    __rlshift__ = _invalid_op('<<')
    __rrshift__ = _invalid_op('>>')
    __neg__ = _invalid_op('-')
    __pos__ = _invalid_op('+')
    __abs__ = _invalid_op('abs')


def _int_enum_flags_unpickler(flags_type, value):
    return int.__new__(flags_type, value)


def _int_enum_flags_pickler(flags):
    return _int_enum_flags_unpickler, (type(flags), int(flags))


def int_enum_flags(name, enum_class, module=None):
    assert issubclass(enum_class, IntEnum)
    if enum_class.__flags_class__ is not None:
        return enum_class.__flags_class__
    dct = {'__module__': module or enum_class.__module__}
    flags_class = type(name, (_IntEnumFlags,), dct)
    flags_class.__enum_class__ = enum_class
    enum_class.__flags_class__ = flags_class
    copy_reg.pickle(flags_class, _int_enum_flags_pickler)
    return flags_class
