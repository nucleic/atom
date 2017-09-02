#------------------------------------------------------------------------------
# Copyright (c) 2013-2017, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
from .catom import Validate
from .list import List


class ContainerList(List):
    """ A List member which supports container notifications.

    """
    __slots__ = ()

    def __init__(self, item=None, default=None):
        """ Initialize a ContainerList.

        """
        super(ContainerList, self).__init__(item, default)
        self.set_validate_mode(Validate.ContainerList, self.item)
