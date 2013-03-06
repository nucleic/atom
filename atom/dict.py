#------------------------------------------------------------------------------
# Copyright (c) 2013, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
from UserDict import DictMixin

from .catom import Member, DefaultValue, Validate, null
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
        if key is not None and not isinstance(key, Member):
            key = Instance(key)
        if value is not None and not isinstance(value, Member):
            value = Instance(value)
        self.set_default_kind(DefaultDict, default)
        self.set_validate_kind(ValidateDict, (key, value))

    def set_member_name(self, name):
        """ Assign the name to this member.

        This method is called by the Atom metaclass when a class is
        created. This makes sure the name of the internal members are
        also updated.

        """
        super(Dict, self).set_member_name(name)
        key, value = self.validate_kind[1]
        if key is not None:
            key.set_member_name(name + '|key')
        if value is not None:
            value.set_member_name(name + '|value')

    def set_member_index(self, index):
        """ Assign the index to this member.

        This method is called by the Atom metaclass when a class is
        created. This makes sure the index of the internal members are
        also updated.

        """
        super(Dict, self).set_member_index(index)
        key, value = self.validate_kind[1]
        if key is not None:
            key.set_member_index(index)
        if value is not None:
            value.set_member_index(index)

    def __get__(self, owner, cls):
        """ Get the dict object for the member.

        If validation is enabled for the dict, a dict proxy will be
        returned which will intercept method calls and perform the
        required validation.

        """
        # XXX move this down to C++ by wrapping the dict in a proxy
        # during the validate method. Although, that will create a
        # reference cycle to the owner, which should be avoided. So
        # maybe wrapping on the fly is the better idea.
        if owner is None:
            return self
        data = super(Dict, self).__get__(owner, cls)
        key, value = self.validate_kind[1]
        if key is None and value is None:
            return data
        return _DictProxy(owner, key, value, data)


class _DictProxy(object, DictMixin):
    """ A private proxy object which validates dict modifications.

    Instances of this class should not be created by user code.

    """
    # XXX move this class down to C++
    def __init__(self, owner, keymember, valmember, data):
        self._owner = owner
        self._keymember = keymember
        self._valmember = valmember
        self._data = data

    def __getitem__(self, key):
        return self._data[key]

    def __setitem__(self, key, value):
        owner = self._owner
        if self._keymember is not None:
            key = self._keymember.do_validate(owner, null, key)
        if self._valmember is not None:
            value = self._valmember.do_validate(owner, null, value)
        self._data[key] = value

    def __delitem__(self, key):
        del self._data[key]

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
