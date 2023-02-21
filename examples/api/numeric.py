# --------------------------------------------------------------------------------------
# Copyright (c) 2013-2023, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
"""Demonstration of the member handling numeric values.

"""
import sys

from atom.api import Atom, Bool, Float, Int


class Data(Atom):
    ival = Int(1)

    lval = Int(sys.maxsize + 1)

    fval = Float(1.5e6)

    bval = Bool(False)


if __name__ == "__main__":
    data = Data()
    print(data.ival)
    print(data.lval)
    print(data.fval)
    print(data.bval)
