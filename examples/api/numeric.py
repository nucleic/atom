#------------------------------------------------------------------------------
# Copyright (c) 2014, Nucleic
#
# Distributed under the terms of the BSD 3-Clause License.
#
# The full license is in the file LICENSE, distributed with this software.
#------------------------------------------------------------------------------
from __future__ import print_function
import sys

from atom.api import Atom, Int, Long, Float, Bool


class Data(Atom):

    ival = Int(1)

    lval = Long(sys.maxint + 1)

    fval = Float(1.5e6)

    bval = Bool(False)


if __name__ == '__main__':
    data = Data()
    print(data.ival)
    print(data.lval)
    print(data.fval)
    print(data.bval)
