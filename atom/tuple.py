#------------------------------------------------------------------------------
# Copyright (c) 2013, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
from __future__ import (division, print_function, absolute_import)

from .catom import Member, DefaultValue, Validate
from .instance import Instance


class Tuple(Member):
    """ A member which allows tuple values.

    If item validation is used, then assignment will create a copy of
    the original tuple before validating the items, since validation
    may change the item values.

    """
    __slots__ = ()

    def __init__(self, item=None, default=()):
        """ Initialize a Tuple.

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
            item = Instance(item)
        self.set_default_value_mode(DefaultValue.Static, default)
        self.set_validate_mode(Validate.Tuple, item)
