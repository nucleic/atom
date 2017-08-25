#------------------------------------------------------------------------------
# Copyright (c) 2013-2017, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
from .catom import Member, GetAttr, SetAttr, DelAttr


class Signal(Member):
    """ A member which acts similar to a Qt signal.

    """
    __slots__ = ()

    def __init__(self):
        """ Initialize a Signal.

        """
        self.set_getattr_mode(GetAttr.Signal, None)
        self.set_setattr_mode(SetAttr.Signal, None)
        self.set_delattr_mode(DelAttr.Signal, None)
