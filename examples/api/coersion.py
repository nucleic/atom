#------------------------------------------------------------------------------
# Copyright (c) 2013, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
from __future__ import (division, unicode_literals, print_function,
                        absolute_import)

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
