# --------------------------------------------------------------------------------------
# Copyright (c) 2013-2023, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
"""Demonstration of the basic use of the Coerced member.

"""
from atom.api import Atom, Coerced


class Demo(Atom):
    # The coercer could be omitted but being explicit helps static type checkers
    cint = Coerced(int, coercer=int)
    cfloat = Coerced(float, coercer=float)
    cstr = Coerced(str, coercer=str)


if __name__ == "__main__":
    demo = Demo()

    print("CInt Demo")
    demo.cint = "1"
    print(demo.cint)
    demo.cint = 51.5
    print(demo.cint)

    print("\nCFloat Demo")
    demo.cfloat = "1.5"
    print(demo.cfloat)
    demo.cfloat = 100
    print(demo.cfloat)

    print("\nCStr Demo")
    demo.cstr = 100
    print(demo.cstr)
    demo.cstr = Demo
    print(demo.cstr)
