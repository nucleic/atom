# --------------------------------------------------------------------------------------
# Copyright (c) 2019-2023, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
from .catom import DefaultValue, Member, Validate
from .instance import Instance
from .typing_utils import extract_types, is_optional


class Set(Member):
    """A member which allows set values.

    Assigning to a set creates a copy. The original set will remain
    unmodified. This is similar to the semantics of the assignment
    operator on the C++ STL container classes.

    """

    __slots__ = "item"

    def __init__(self, item=None, default=None):
        """Initialize a Set.

        Parameters
        ----------
        item : Member, type, or tuple of types, optional
            A member to use for validating the types of items allowed in
            the set. This can also be a type object or a tuple of types,
            in which case it will be wrapped with an Instance member. If
            this is not given, no item validation is performed.

        default : list, optional
            The default list of values. A new copy of this list will be
            created for each atom instance.

        """
        self.set_default_value_mode(DefaultValue.Set, default)
        if item is not None and not isinstance(item, Member):
            opt, types = is_optional(extract_types(item))
            item = Instance(types, optional=opt)
        self.item = item
        self.set_validate_mode(Validate.Set, item)

    def set_name(self, name):
        """Assign the name to this member.

        This method is called by the Atom metaclass when a class is
        created. This makes sure the name of the internal members are
        also updated.

        """
        super(Set, self).set_name(name)
        item = self.item
        if item is not None:
            item.set_name(name + "|item")

    def set_index(self, index):
        """Assign the index to this member.

        This method is called by the Atom metaclass when a class is
        created. This makes sure the index of the internal members are
        also updated.

        """
        super(Set, self).set_index(index)
        item = self.item
        if item is not None:
            item.set_index(index)

    def clone(self):
        """Create a clone of the member.

        This will clone the internal set item members if they exist.

        """
        clone = super(Set, self).clone()
        item = self.item
        if item is not None:
            clone.item = item_clone = item.clone()
            mode, ctxt = self.validate_mode
            clone.set_validate_mode(mode, item_clone)
        else:
            clone.item = None
        return clone
