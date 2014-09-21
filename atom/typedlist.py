#------------------------------------------------------------------------------
# Copyright (c) 2014, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
from .catom import CTypedList
from .errors import ValidationError
from .formatting import kind_repr


class TypedList(CTypedList):
    """ A custom list class which strongly types its elements.

    Parameters
    ----------
    value_type : type or tuple of types
        The allowed type of the values in the list. This has the same
        semantics as the second argument to the builtin isinstance().

    sequence : iterable, optional
        The initial values to add to the list.

    """
    __slots__ = ()

    def validation_error(self, value):
        """ Raise a ValidationError for the given value.

        This is called by the C++ layer when value validation fails.

        This method may be reimplemented by subclasses if necessary
        in order to raise a custom ValidationError.

        Parameters
        ----------
        value : object
            The value which failed validation.

        Raises
        ------
        ValidationError

        """
        type_repr = kind_repr(self.value_type)
        value_repr = '%r %r' % (value, type(value))
        msg = ("Each element of the list must be an instance of %s, "
               "but a value of %s was specified.") % (type_repr, value_repr)
        raise ValidationError(msg)
