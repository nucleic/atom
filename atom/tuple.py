# --------------------------------------------------------------------------------------
# Copyright (c) 2013-2024, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
from typing import Tuple as TTuple

from .catom import DefaultValue, Member, Validate
from .instance import Instance
from .typing_utils import extract_types, is_optional


class Tuple(Member):
    """A member which allows tuple values.

    If item validation is used, then assignment will create a copy of
    the original tuple before validating the items, since validation
    may change the item values.

    """

    __slots__ = ("item",)

    def __init__(self, item=None, default=()):
        """Initialize a Tuple.

        Parameters
        ----------
        item : Member, type, or tuple of types, optional
            A member to use for validating the types of items allowed in
            the tuple. This can also be a type object or a tuple of types,
            in which case it will be wrapped with an Instance member. If
            this is not given, no item validation is performed.

        default : tuple, optional
            The default tuple of values.

        """
        if item is not None and not isinstance(item, Member):
            opt, types = is_optional(extract_types(item))
            item = Instance(types, optional=opt)
        self.item = item
        self.set_default_value_mode(DefaultValue.Static, default)
        self.set_validate_mode(Validate.Tuple, item)

    def set_name(self, name):
        """Set the name of the member.

        This method ensures that the item member name is also updated.

        """
        super(Tuple, self).set_name(name)
        if self.item is not None:
            self.item.set_name(name + "|item")

    def set_index(self, index):
        """Assign the index to this member.

        This method ensures that the item member index is also updated.

        """
        super(Tuple, self).set_index(index)
        if self.item is not None:
            self.item.set_index(index)

    def clone(self):
        """Create a clone of the tuple.

        This will clone the internal tuple item if one is in use.

        """
        clone = super(Tuple, self).clone()
        item = self.item
        if item is not None:
            clone.item = item_clone = item.clone()
            mode, ctxt = self.validate_mode
            clone.set_validate_mode(mode, item_clone)
        else:
            clone.item = None
        return clone


class FixedTuple(Member):
    """A member which allows tuple values with a fixed number of items.

    Items are always validated and can be of different types.
    Assignment will create a copy of the original tuple before validating the
    items, since validation may change the item values.

    """

    #: Members used to validate each element of the tuple.
    items: TTuple[Member, ...]

    __slots__ = ("items",)

    def __init__(self, *items, default=None):
        """Initialize a Tuple.

        Parameters
        ----------
        items : Iterable[Member | type | tuple[type, ...]]
            A member to use for validating the types of items allowed in
            the tuple. This can also be a type object or a tuple of types,
            in which case it will be wrapped with an Instance member.

        default : tuple, optional
            The default tuple of values.

        """
        mitems = []
        for i in items:
            if not isinstance(i, Member):
                opt, types = is_optional(extract_types(i))
                i = Instance(types, optional=opt)
            mitems.append(i)

        self.items = mitems

        if default is None:
            self.set_default_value_mode(DefaultValue.NonOptional, None)
        else:
            self.set_default_value_mode(DefaultValue.Static, default)
        self.set_validate_mode(Validate.FixedTuple, tuple(mitems))

    def set_name(self, name):
        """Set the name of the member.

        This method ensures that the item member name is also updated.

        """
        super().set_name(name)
        for i, item in enumerate(self.items):
            item.set_name(name + f"|item_{i}")

    def set_index(self, index):
        """Assign the index to this member.

        This method ensures that the item member index is also updated.

        """
        super().set_index(index)
        for item in self.items:
            item.set_index(index)

    def clone(self):
        """Create a clone of the tuple.

        This will clone the internal tuple item if one is in use.

        """
        clone = super().clone()
        clone.items = items_clone = tuple(i.clone() for i in self.items)
        mode, _ = self.validate_mode
        clone.set_validate_mode(mode, items_clone)
        return clone
