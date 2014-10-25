#------------------------------------------------------------------------------
# Copyright (c) 2014, Nucleic
#
# Distributed under the terms of the BSD 3-Clause License.
#
# The full license is in the file LICENSE, distributed with this software.
#------------------------------------------------------------------------------
from __future__ import print_function

from atom.api import Atom, Coerced


class Demo(Atom):

    cint = Coerced(int)
    cfloat = Coerced(float)
    cstr = Coerced(str)


if __name__ == '__main__':
    demo = Demo()

    print('CInt Demo')
    demo.cint = '1'
    print(demo.cint)
    demo.cint = 51.5
    print(demo.cint)

    print('\nCFloat Demo')
    demo.cfloat = '1.5'
    print(demo.cfloat)
    demo.cfloat = 100
    print(demo.cfloat)

    print('\nCStr Demo')
    demo.cstr = 100
    print(demo.cstr)
    demo.cstr = Demo
    print(demo.cstr)
