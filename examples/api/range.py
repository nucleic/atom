# --------------------------------------------------------------------------------------
# Copyright (c) 2013-2023, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
"""Demonstration of the ranges members.

"""
from atom.api import Atom, FloatRange, Range


class Experiment(Atom):
    coef = FloatRange(-1.0, 1.0, 0.0)

    gain = Range(0, 100, 10)

    scale = FloatRange(0.0, 2.0, 1.0, strict=True)


if __name__ == "__main__":
    exp = Experiment()

    print(exp.coef)
    exp.coef = 0.5
    print(exp.coef)

    print(exp.gain)
    exp.gain = 99
    print(exp.gain)

    print(exp.scale)
    try:
        exp.scale = 2  # strict=False prevents assigning int/long
    except TypeError as e:
        print(e)
        exp.scale = 2.0
    print(exp.scale)
