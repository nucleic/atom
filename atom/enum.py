#------------------------------------------------------------------------------
# Copyright (c) 2013-2017, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
from .catom import Member, DefaultValue, Validate


class Enum(Member):
    """ A member where the value can be one in a sequence of items.

    """
    __slots__ = ()

    def __init__(self, *items):
        """ Initialize an Enum.

        Parameters
        ----------
        *items
            The allowed values which can be assigned to the enum.

        """
        if len(items) == 0:
            raise ValueError('an Enum requires at least 1 item')
        self.set_default_value_mode(DefaultValue.Static, items[0])
        self.set_validate_mode(Validate.Enum, items)

    @property
    def items(self):
        """ A readonly property which returns the items in the enum.

        """
        return self.validate_mode[1]

    def added(self, *items):
        """ Create a clone of the Enum with added items.

        Parameters
        ----------
        *items
            Additional items to include in the Enum.

        Returns
        -------
        result : Enum
            A new enum object which contains all of the original items
            plus the new items.

        """
        olditems = self.items
        newitems = olditems + items
        clone = self.clone()
        clone.set_validate_mode(Validate.Enum, newitems)
        return clone

    def removed(self, *items):
        """ Create a clone of the Enum with some items removed.

        Parameters
        ----------
        *items
            The items to remove remove from the new enum.

        Returns
        -------
        result : Enum
            A new enum object which contains all of the original items
            but with the given items removed.

        """
        newitems = tuple(i for i in self.items if i not in items)
        if len(newitems) == 0:
            raise ValueError('an Enum requires at least 1 item')
        clone = self.clone()
        clone.set_default_value_mode(DefaultValue.Static, newitems[0])
        clone.set_validate_mode(Validate.Enum, newitems)
        return clone

    def __call__(self, item):
        """ Create a clone of the Enum item with a new default.

        Parameters
        ----------
        item : object
            The item to use as the Enum default. The item must be one
            of the valid enum items.

        """
        if item not in self.items:
            raise TypeError('invalid enum value')
        clone = self.clone()
        clone.set_default_value_mode(DefaultValue.Static, item)
        return clone
