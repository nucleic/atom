# --------------------------------------------------------------------------------------
# Copyright (c) 2013-2023, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
"""Demonstration of the member handling containers.

"""
from atom.api import Atom, ContainerList, Dict, List, Tuple


class Data(Atom):
    dlist = List(default=[1, 2, 3])

    dcont_list = ContainerList(default=[1, 2, 3])

    dtuple = Tuple(default=(5, 4, 3))

    ddict = Dict(default={"foo": 1, "bar": "a"})

    def _observe_dcont_list(self, change):
        print("container list change: {0}".format(change["value"]))


if __name__ == "__main__":
    data = Data()
    print(data.dlist)
    print(data.dcont_list)
    data.dcont_list.append(1)
    data.dcont_list.pop(0)
    print(data.dtuple)
    print(data.ddict)
