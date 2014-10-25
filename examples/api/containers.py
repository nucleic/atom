#------------------------------------------------------------------------------
# Copyright (c) 2014, Nucleic
#
# Distributed under the terms of the BSD 3-Clause License.
#
# The full license is in the file LICENSE, distributed with this software.
#------------------------------------------------------------------------------
from __future__ import print_function

from atom.api import Atom, List, ContainerList, Tuple, Dict


class Data(Atom):

    dlist = List(default=[1, 2, 3])

    dcont_list = ContainerList(default=[1, 2, 3])

    dtuple = Tuple(default=(5, 4, 3))

    ddict = Dict(default=dict(foo=1, bar='a'))

    def _observe_dcont_list(self, change):
        print('container list change: {0}'.format(change['value']))


if __name__ == '__main__':
    data = Data()
    print(data.dlist)
    print(data.dcont_list)
    data.dcont_list.append(1)
    data.dcont_list.pop(0)
    print(data.dtuple)
    print(data.ddict)
