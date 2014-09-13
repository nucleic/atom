#------------------------------------------------------------------------------
# Copyright (c) 2014, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
import sys

from .catom import ValidationError
from .formatting import container_message


class ListMeta(type):
    """ A metaclass for creating strongly typed lists.

    """
    #: A cache which maps a value type to the typed list type.
    #: This should not be manipulated directly by user code.
    type_cache = {}

    def __getitem__(self, value_type):
        """ Get or create a typed list subclass for a value type.

        Parameters
        ----------
        value_type : type
            The allowed value type for the elements in the list.

        """
        list_type = self.type_cache.get(value_type)
        if list_type is not None:
            return list_type
        if not isinstance(value_type, type):
            msg = "value_type must be a type, not '%s'"
            raise TypeError(msg % type(value_type).__name__)
        name = 'List[%s]' % value_type.__name__
        class_dict = {'__slots__': (), 'value_type': value_type}
        list_type = ListMeta(name, (List,), class_dict)
        self.type_cache[value_type] = list_type
        return list_type


class List(list):
    """ A list subclass which strongly enforces element types.

    Strongly typed lists are created by indexing the `List` class with
    the element value type:

        IntList = List[int]

    That new type can then be used like any other Python list:

        my_int_list = IntList()
        my_int_list.append(1)
        my_int_list.append(2.0)  # validation error: float != int

    The generated list types are cached, so List[int] always returns
    the same type. This guarantees the behavior:

        isinstance(my_int_list, List[int])  # True
        isinstance(my_int_list, list)       # True
        isinstance([1, 2], List[int])       # False

    """
    __metaclass__ = ListMeta

    __slots__ = ()

    #: The allowed value type for the elements in the list.
    #: This should not be manipulated directly by user code.
    value_type = object

    def __init__(self, sequence=None):
        """ Initialize the typed list object.

        Parameters
        ----------
        sequence : iterable, optional
            An iterable of initial values for the list.

        """
        if len(self) > 0:
            del self[:]
        if sequence is not None:
            self.extend(sequence)

    def __setitem__(self, index, value):
        """ Set the value of the list at the given index.

        Parameters
        ----------
        index : int or slice
            The index or slice of indices in the list.

        value : value_type or iterable of value_type
            The value or iterable of values to assign to the index.

        """
        validate = self.validate
        if isinstance(index, slice):
            value = [validate(val) for val in value]
        else:
            value = validate(value)
        super(List, self).__setitem__(index, value)

    if sys.version_info[0] < 3:

        def __setslice__(self, i, j, iterable):
            self.__setitem__(slice(i, j), iterable)

    # TODO __iadd__
    # TODO __imul__

    def append(self, value):
        """ Append a value to the list.

        Parameters
        ----------
        value : value_type
            The value to add to the end of the list.

        """
        value = self.validate(value)
        super(List, self).append(value)

    def extend(self, iterable):
        """ Extend the list by appending values to the end.

        Parameters
        ----------
        iterable : iterable of value_type
            The iterable of values to add to the end of the list.

        """
        validate = self.validate
        values = [validate(value) for value in iterable]
        super(List, self).extend(values)

    def insert(self, index, value):
        """ Insert a value at the given index.

        Parameters
        ----------
        index : int
            The postion at which to insert the value.

        value : value_type
            The value to insert into the list.

        """
        value = self.validate(value)
        super(List, self).insert(index, value)

    def validate(self, value):
        """ Validate that a value has the correct type.

        The default implementation of this method checks that the
        value is an instance of `value_type`.

        This method can be overridden to implement custom validation.

        Parameters
        ----------
        value : value_type
            The value to validate.

        Returns
        -------
        result : value_type
            The validated value.

        Raises
        ------
        ValidationError
            The value is not an instance of `value_type`.

        """
        if isinstance(value, self.value_type):
            return value
        msg = container_message(self, self.value_type, 'element', value)
        raise ValidationError(msg)
