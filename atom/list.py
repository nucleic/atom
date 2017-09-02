#------------------------------------------------------------------------------
# Copyright (c) 2013-2017, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
from .catom import Member, DefaultValue, Validate
from .instance import Instance
from .typed import Typed


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
            item = Typed(item) if isinstance(item, type) else Instance(item)
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
