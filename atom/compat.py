#------------------------------------------------------------------------------
# Copyright (c) 2013-2018, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
import sys

IS_PY3 = sys.version_info >= (3,)


# From future.utils
def with_metaclass(meta, *bases):
    """
    Function from jinja2/_compat.py. License: BSD.

    Use it like this::

        class BaseForm(object):
            pass

        class FormType(type):
            pass

        class Form(with_metaclass(FormType, BaseForm)):
            pass

    This requires a bit of explanation: the basic idea is to make a
    dummy metaclass for one level of class instantiation that replaces
    itself with the actual metaclass.  Because of internal type checks
    we also need to make sure that we downgrade the custom metaclass
    for one level to something closer to type (that's why __call__ and
    __init__ comes back from type etc.).

    This has the advantage over six.with_metaclass of not introducing
    dummy classes into the final MRO.
    """
    class MetaClass(meta):
        __call__ = type.__call__
        __init__ = type.__init__

        def __new__(cls, name, this_bases, d):
            if this_bases is None:
                return type.__new__(cls, name, (), d)
            return meta(name, bases, d)
    return MetaClass('temporary_class', None, {})


if IS_PY3:
    import builtins
    bytes = builtins.bytes
    int = builtins.int
    str = builtins.str
    long = int

    # Alternatively this could be used
    # From past.types.basestring
    class BaseBaseString(type):
        def __instancecheck__(cls, instance):
            return isinstance(instance, (bytes, str))

    def __subclasshook__(cls, thing):
        raise NotImplemented  # pragma: no cover

    class basestring(with_metaclass(BaseBaseString)):
        """
        A minimal backport of the Python 2 basestring type to Py3
        """


else:
    from __builtin__ import bytes, basestring, int, long
    str = unicode

__all__ = ('IS_PY3', 'with_metaclass', 'int', 'long', 'bytes', 'str',
           'basestring')
