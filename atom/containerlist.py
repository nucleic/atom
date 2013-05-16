#------------------------------------------------------------------------------
# Copyright (c) 2013, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
from .catom import Member, DefaultValue, Validate
from .list import List#, ListProxy
from .instance import Instance
from .typed import Typed


class ContainerList(List):
    """ A List member which supports container notifications.

    """
    __slots__ = ()

    def __init__(self, item=None, default=None):
        """ Initialize a ContainerList.

        """
        if item is not None and not isinstance(item, Member):
            if isinstance(item, type):
                item = Typed(item)
            else:
                item = Instance(item)
        self.item = item
        self.set_default_value_mode(DefaultValue.List, default)
        self.set_validate_mode(Validate.ContainerList, item)
