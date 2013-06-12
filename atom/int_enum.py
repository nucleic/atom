#------------------------------------------------------------------------------
# Copyright (c) 2013, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
# Note: This module is imported by 'atom.catom' module from code defined in
# the 'enumtypes.cpp' file. This module must therefore not import atom.
class IntEnumMeta(type):
    """ The metaclass for IntEnum and its subclasses.

    """
    def __new__(meta, name, bases, dct):
        cls = type.__new__(meta, name, bases, dct)
        enums = {}
        reved = {}
        skip = (int, object)
        for t in reversed(cls.mro()):
            if t in skip:
                continue
            for key, value in t.__dict__.iteritems():
                if key in enums and enums[key] != value:
                    msg = "conflicting enum value for name '%s'"
                    raise TypeError(msg % key)
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
        enum_values = cls.__enums__.values()
        enum_values.sort()
        return iter(enum_values)

    def __setattr__(cls, name, value):
        if name in cls.__enums__:
            raise TypeError("cannot change the value of an enum")
        type.__setattr__(cls, name, value)


class IntEnum(int):
    """ An integer subclass for declaring enum types.

    """
    __metaclass__ = IntEnumMeta

    def __reduce_ex__(self, proto):
        """ Reduce the enum value for pickling.

        """
        return (type(self), (int(self),))

    def __repr__(self):
        """ Get a string representation of the enum.

        """
        t_name = type(self).__name__
        s_name = self.__enum_name__
        return '<enum: %s.%s [value=%d]>' % (t_name, s_name, self)

    def __str__(self):
        """ Get a print string representation of the enum.

        """
        return '%s.%s' % (type(self).__name__, self.__enum_name__)

    @property
    def name(self):
        """ Get the name associated with this enum value.

        """
        return self.__enum_name__
