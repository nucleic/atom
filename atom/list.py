#------------------------------------------------------------------------------
# Copyright (c) 2013, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
from .catom import Member, PostGetAttr, DefaultValue, Validate
from .instance import Instance


class List(Member):
    """ A member which allows list values.

    Assigning to a list creates a copy. The orginal list will remain
    unmodified. This is similar to the semantics of the assignment
    operator on the C++ STL container classes.

    """
    __slots__ = 'item'

    def __init__(self, item=None, default=None, copy=True):
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

        copy : bool, optional
            Whether the list should be copied on assignment so that the
            member has full ownership of the list. If this is False, it
            is possible for validation to mutate the original list in
            place. The default is True.

        """
        if item is not None and not isinstance(item, Member):
            item = Instance(item)
        self.item = item
        self.set_default_value_mode(DefaultValue.List, default)
        if copy:
            self.set_validate_mode(Validate.List, item)
        else:
            self.set_validate_mode(Validate.ListNoCopy, item)

        # Only use the post getattr handler if there is a validator
        # item. This prevents unneeded creation of ListProxy objects.
        if item is not None:
            self.set_post_getattr_mode(
                PostGetAttr.MemberMethod_ObjectValue, "post_getattr"
            )

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

    def post_getattr(self, owner, value):
        """ A post getattr handler.

        This handler is only invoked if the list uses an item validator.

        """
        return ListProxy(self, owner, value)


class ListProxy(object):
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

    def __add__(self, other):
        return self._value + other

    def __call__(self):
        """ Retrieve the raw list object wrapped by the proxy.

        """
        return self._value

    def __contains__(self, item):
        return item in self._value

    def __delitem__(self, index):
        del self._value[index]

    def __eq__(self, other):
        return self._value == other

    def __ge__(self, other):
        return self._value >= other

    def __getitem__(self, index):
        return self._value[index]

    def __gt__(self, other):
        return self._value > other

    def __hash__(self, other):
        return hash(self._value)

    def __iadd__(self, items):
        validator = self._member.item
        if validator is not None:
            owner = self._owner
            validate = validator.do_full_validate
            items = [validate(owner, None, i) for i in items]
        self._value += items
        return self._value

    def __imul__(self, count):
        self._value *= count
        return self._value

    def __iter__(self):
        return iter(self._value)

    def __le__(self, other):
        return self._value <= other

    def __len__(self):
        return len(self._value)

    def __lt__(self, other):
        return self._value < other

    def __mul__(self, other):
        return self._value * other

    def __ne__(self, other):
        return self._value != other

    def __repr__(self):
        return repr(self._value)

    def __rmul__(self, other):
        return other * self._value

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

    def __str__(self):
        return str(self._value)

    def append(self, item):
        validator = self._member.item
        if validator is not None:
            validate = validator.do_full_validate
            item = validate(self._owner, None, item)
        self._value.append(item)

    def count(self, item):
        return self._value.count(item)

    def extend(self, items):
        validator = self._member.item
        if validator is not None:
            owner = self._owner
            validate = validator.do_full_validate
            items = [validate(owner, None, i) for i in items]
        self._value.extend(items)

    def index(self, item):
        return self._value.index(item)

    def insert(self, index, item):
        validator = self._member.item
        if validator is not None:
            validate = validator.do_full_validate
            item = validate(self._owner, None, item)
        self._value.insert(index, item)

    def pop(self, *args):
        return self._value.pop(*args)

    def remove(self, item):
        self._value.remove(item)

    def reverse(self):
        self._value.reverse()

    def sort(self, cmp=None, key=None, reverse=False):
        self._value.sort(cmp, key, reverse)
