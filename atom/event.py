#------------------------------------------------------------------------------
# Copyright (c) 2013, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
from .catom import Member, Validate, GetAttr, SetAttr


class Event(Member):
    """ A member which acts like a stateless event.

    """
    __slots__ = ()

    def __init__(self, kind=None):
        """ Initialize an Event.

        Parameters
        ----------
        kind : type or Member, optional
            The type of argument which may be emitted by the event or
            a Member which will validate the argument which can be
            emitted. The default is None and indicates no validation
            will be performed.

        """
        self.set_getattr_mode(GetAttr.Event, None)
        self.set_setattr_mode(SetAttr.Event, None)
        if kind is not None:
            if isinstance(kind, Member):
                self.set_validate_mode(Validate.Delegate, kind)
            else:
                self.set_validate_mode(Validate.Instance, kind)
