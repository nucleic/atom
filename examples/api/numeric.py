#------------------------------------------------------------------------------
# Copyright (c) 2013, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
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
