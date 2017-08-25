#------------------------------------------------------------------------------
# Copyright (c) 2013-2017, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
import sys
if sys.version_info >= (3,):
    from collections import MutableMapping
else:
    from UserDict import DictMixin as MutableMapping

from .catom import Member, PostGetAttr, DefaultValue, Validate
from .instance import Instance


class Dict(Member):
    """ A value of type `dict`.

    """
    __slots__ = ()

    def __init__(self, key=None, value=None, default=None):
        """ Initialize a Dict.

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

        """
        self.set_default_value_mode(DefaultValue.Dict, default)
        if key is not None and not isinstance(key, Member):
            key = Instance(key)
        if value is not None and not isinstance(value, Member):
            value = Instance(value)
        self.set_validate_mode(Validate.Dict, (key, value))
        if key is not None or value is not None:
            mode = PostGetAttr.MemberMethod_ObjectValue
            self.set_post_getattr_mode(mode, 'post_getattr')

    def post_getattr(self, owner, data):
        """ A post getattr handler.

        If the dict performs key or value validation, then this handler
        will be called to wrap the dict in a proxy object on the fly.

        """
        key, value = self.validate_mode[1]
        return _DictProxy(owner, key, value, data)

    def set_name(self, name):
        """ Assign the name to this member.

        This method is called by the Atom metaclass when a class is
        created. This makes sure the name of the internal members are
        also updated.

        """
        super(Dict, self).set_name(name)
        key, value = self.validate_mode[1]
        if key is not None:
            key.set_name(name + '|key')
        if value is not None:
            value.set_name(name + '|value')

    def set_index(self, index):
        """ Assign the index to this member.

        This method is called by the Atom metaclass when a class is
        created. This makes sure the index of the internal members are
        also updated.

        """
        super(Dict, self).set_index(index)
        key, value = self.validate_mode[1]
        if key is not None:
            key.set_index(index)
        if value is not None:
            value.set_index(index)


class _DictProxy(MutableMapping):
    """ A private proxy object which validates dict modifications.

    Instances of this class should not be created by user code.

    """
    # XXX move this class to C++
    def __init__(self, owner, keymember, valmember, data):
        self._owner = owner
        self._keymember = keymember
        self._valmember = valmember
        self._data = data

    def __repr__(self):
        return repr(self._data)

    def __getitem__(self, key):
        return self._data[key]

    def __setitem__(self, key, value):
        owner = self._owner
        if self._keymember is not None:
            key = self._keymember.do_full_validate(owner, None, key)
        if self._valmember is not None:
            value = self._valmember.do_full_validate(owner, None, value)
        self._data[key] = value

    def __delitem__(self, key):
        del self._data[key]

    def __len__(self):
        return len(self._data)

    def __iter__(self):
        return iter(self._data)

    def __contains__(self, key):
        return key in self._data

    def keys(self):
        return self._data.keys()

    def copy(self):
        return self._data.copy()

    def has_key(self, key):
        return key in self._data
