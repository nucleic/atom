#------------------------------------------------------------------------------
# Copyright (c) 2013, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
from .catom import Member, DefaultValue, Validate


class Type(Member):
    """ A value which allows subclass objects of a given type.

    Values will be tested using the `PyObject_IsSubclass C API call.

    """
    __slots__ = ()

    def __init__(self, kind):
        """ Initialize an Typed.

        Parameters
        ----------
        kind : type
            The allowed type for the value.
            
        """
        self.set_validate_mode(Validate.Type, kind)
