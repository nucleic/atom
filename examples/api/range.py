#------------------------------------------------------------------------------
# Copyright (c) 2014, Nucleic
#
# Distributed under the terms of the BSD 3-Clause License.
#
# The full license is in the file LICENSE, distributed with this software.
#------------------------------------------------------------------------------
from __future__ import print_function

from atom.api import Atom, Range, FloatRange


class Experiment(Atom):

    coef = FloatRange(-1.0, 1.0, 0.0)

    gain = Range(0, 100, 10)


if __name__ == '__main__':
    exp = Experiment()

    print(exp.coef)
    exp.coef = 0.5
    print(exp.coef)

    print(exp.gain)
    exp.gain = 99
    print(exp.gain)
