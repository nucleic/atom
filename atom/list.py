#------------------------------------------------------------------------------
# Copyright (c) 2013, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
from .catom import Member, DefaultValue, Validate
from .instance import Instance


class List(Member):
    """ A member which allows list values.

    Assigning to a list creates a copy. The orginal list will remain
    unmodified. This is similar to the semantics of the assignment
    operator on the C++ STL container classes.

    """
    __slots__ = 'item'

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
        self.item = item
        self.set_default_value_mode(DefaultValue.List, default)
        self.set_validate_mode(Validate.List, item)

    def set_name(self, name):
        """ Set the name of the member.

        This method ensures that the item member name is also updated.

        """
        super(List, self).set_name(name)
        if self.item is not None:
            self.item.set_name(name + "|item")

    def set_index(self, index):
        """ Assign the index to this member.

        This method ensures that the item member index is also updated.

        """
        super(List, self).set_index(index)
        if self.item is not None:
            self.item.set_index(index)

    def clone(self):
        """ Create a clone of the list.

        This will clone the internal list item if one is in use.

        """
        clone = super(List, self).clone()
        item = self.item
        if item is not None:
            clone.item = item_clone = item.clone()
            mode, ctxt = self.validate_mode
            clone.set_validate_mode(mode, item_clone)
        else:
            clone.item = None
        return clone


class ListProxy(list):
    """ A proxy object which validates in-place list operations.

    Instances of this class are created on the fly by the post getattr
    handler of a List which has an item validator.

    """
    # TODO move this class to C++
    __slots__ = ('_member', '_owner', '_value')

    def __init__(self, member, owner, value):
        """ Initialize a ProxyList.

        Parameters
        ----------
        member : List
            The List member which created this ListProxy.

        owner : Atom
            The atom object which owns the list value.

        value : list
            The data value for the member.

        """
        self._member = member
        self._owner = owner
        self._value = value

    def __iadd__(self, items):
        validator = self._member.item
        if validator is not None:
            owner = self._owner
            validate = validator.do_full_validate
            items = [validate(owner, None, i) for i in items]
        self._value += items
        return self._value

    def __setitem__(self, index, item):
        validator = self._member.item
        if validator is not None:
            owner = self._owner
            validate = validator.do_full_validate
            if isinstance(index, slice):
                item = [validate(owner, None, i) for i in item]
            else:
                item = validate(owner, None, item)
        self._value[index] = item

    def append(self, item):
        validator = self._member.item
        if validator is not None:
            validate = validator.do_full_validate
            item = validate(self._owner, None, item)
        self._value.append(item)

    def extend(self, items):
        validator = self._member.item
        if validator is not None:
            owner = self._owner
            validate = validator.do_full_validate
            items = [validate(owner, None, i) for i in items]
        self._value.extend(items)

    def insert(self, index, item):
        validator = self._member.item
        if validator is not None:
            validate = validator.do_full_validate
            item = validate(self._owner, None, item)
        self._value.insert(index, item)
