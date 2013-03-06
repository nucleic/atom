#------------------------------------------------------------------------------
# Copyright (c) 2013, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
from .catom import Member, null, PostGetAttr, DefaultValue, Validate
from .instance import Instance


class List(Member):
    """ A member which allows list values.

    Assigning to a list creates a copy. The orginal list will remain
    unmodified. This is similar to the semantics of the assignment
    operator on the C++ STL container classes.

    """
    __slots__ = ()

    def __init__(self, item=None, default=None):
        """ Initialize a List.

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

        """
        if item is not None and not isinstance(item, Member):
            item = Instance(item)
        self.set_default_value_mode(DefaultValue.List, default)
        self.set_validate_mode(Validate.List, item)
        if item is not None:
            mode = PostGetAttr.MemberMethod_ObjectValue
            self.set_post_getattr_mode(mode, 'post_getattr')

    def post_getattr(self, owner, data):
        """ A post getattr handler.

        If the list performs item validation, then this handler will
        be called to wrap the list in a proxy object on the fly.

        """
        member = self.validate_mode[1]
        return _ListProxy(owner, member, data)

    def set_name(self, name):
        """ Assign the name to this member.

        This method is called by the Atom metaclass when a class is
        created. This makes sure the name of the internal member is
        also updated.

        """
        super(List, self).set_name(name)
        member = self.validate_mode[1]
        if member is not None:
            member.set_name(name + "|item")

    def set_index(self, index):
        """ Assign the index to this member.

        This method is called by the Atom metaclass when a class is
        created. This makes sure the index of the internal member is
        also updated.

        """
        super(List, self).set_index(index)
        member = self.validate_mode[1]
        if member is not None:
            member.set_index(index)


class _ListProxy(object):
    """ A private proxy object which validates list modifications.

    Instances of this class should not be created by user code.

    """
    # XXX move this class to C++
    def __init__(self, owner, member, data):
        self._owner = owner
        self._member = member
        self._data = data

    def __repr__(self):
        return repr(self._data)

    def __call__(self):
        return self._data

    def __iter__(self):
        return iter(self._data)

    def __getitem__(self, index):
        return self._data[index]

    def __setitem__(self, index, item):
        item = self._member.do_full_validate(self._owner, null, item)
        self._data[index] = item

    def __delitem__(self, index):
        del self._data[index]

    def append(self, item):
        item = self._member.do_full_validate(self._owner, null, item)
        self._data.append(item)

    def insert(self, index, item):
        item = self._member.do_full_validate(self._owner, null, item)
        self._data.insert(index, item)

    def extend(self, items):
        owner = self._owner
        validate = self._member.do_full_validate
        items = [validate(owner, null, item) for item in items]
        self._data.extend(items)

    def pop(self, *args):
        self._data.pop(*args)

    def remove(self, item):
        self._data.remove(item)

    def reverse(self):
        self._data.reverse()

    def sort(self, *args, **kwargs):
        self._data.sort(*args, **kwargs)

