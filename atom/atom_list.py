#------------------------------------------------------------------------------
# Copyright (c) 2014, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
import sys
from weakref import ref

from .catom import ValidationError
from . import formatting


# IMPORTANT!
# This class is imported by member.cpp when needed. Do not move or
# rename this class or its module unless also changing the C++ code.
class AtomList(list):
    """ A list subclass which implements member type checking.

    Instances of this class are created automatically by the Atom C++
    layer. They should not typically be created by user code.

    """
    __slots__ = ('_member', '_atomref', '_name')

    def __init__(self, items, member, atom, name):
        """ Initialize an AtomList.

        Parameters
        ----------
        items : iterable
            The initial list of items for the list. These are *not*
            validated. The caller is responsible for ensuring that
            the initial items are valid.

        member : Member
            The member which created this list.

        validator : Member
            The item validator to use for validating list items.

        atom : Atom
            The atom object which owns the list. Only a weak reference
            to this Atom object is maintained internally.

        """
        super(AtomList, self).__init__(items)
        self._member = member
        self._atomref = ref(atom)
        self._name = name

    @staticmethod
    def _validate(member, atom, name, value):
        """ An internal validation implementation method.

        """
        try:
            value = member.do_validate(atom, name, value)
        except ValidationError:
            type_info = member.type_info
            msg = formatting.element_message(atom, name, value, type_info)
            raise ValidationError(msg)
        return value

    def _validate_single(self, item):
        """ An internal validation method for a single item.

        """
        return self._validate(self._member, self._atomref(), self._name, item)

    def _validate_iterable(self, items):
        """ An internal validation method for an iterable of items.

        """
        member = self._member
        atom = self._atomref()
        name = self._name
        validate = self._validate
        return (validate(member, atom, name, item) for item in items)

    def append(self, value):
        """ Append a value to the end of the list.

        Parameters
        ----------
        value : object
            A value of an allowed type for the list.

        """
        value = self._validate_single(value)
        super(AtomList, self).append(value)

    def insert(self, index, value):
        """ Insert an item into the list at the given index.

        Parameters
        ----------
        index : int
            The index at which to insert the value.

        value : object
            A value of an allowed type for the list.

        """
        value = self._validate_single(value)
        super(AtomList, self).insert(index, value)

    def extend(self, values):
        """ Add values to the end of the list.

        Parameters
        ----------
        values : iterable
            An iterable of values to add to the end of the list.

        """
        values = self._validate_iterable(values)
        super(AtomList, self).extend(values)

    def __setitem__(self, key, value):
        """ Set an item or a slice in the list.

        """
        if isinstance(key, slice):
            value = self._validate_iterable(value)
        else:
            value = self._validate_single(value)
        super(AtomList, self).__setitem__(key, value)

    def __iadd__(self, values):
        """ Add items to the list in-place.

        """
        values = self._validate_iterable(values)
        return super(AtomList, self).__iadd__(values)

    def __reduce_ex__(self, protocol):
        """ Reduce the AtomList for pickling.

        """
        # An AtomList is pickled as a normal list. When the Atom class
        # is reconstituted, assigning the list to the attribute will
        # create a new AtomList with the proper owner. There is no need
        # to try to persist the member or atom information.
        return (list, (tuple(self),))

    if sys.version_info[0] <= 2:

        def __setslice__(self, i, j, values):
            """ Set a slice in the list.

            """
            self.__setitem__(slice(i, j), values)
