#------------------------------------------------------------------------------
# Copyright (c) 2013-2017, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
#------------------------------------------------------------------------------
"""Hello world example: how to write an atom class.

"""
from __future__ import (division, unicode_literals, print_function,
                        absolute_import)

from atom.api import Atom, Str


class Hello(Atom):

    message = Str('Hello')


if __name__ == "__main__":
    hello = Hello()
    print(hello.message)
    hello.message = 'Goodbye'
    print(hello.message)
