#------------------------------------------------------------------------------
# Copyright (c) 2013, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
from __future__ import (division, unicode_literals, print_function,
                        absolute_import)

from atom.api import Atom, Unicode


class Hello(Atom):

    message = Unicode('Hello')


if __name__ == "__main__":
    hello = Hello()
    print(hello.message)
    hello.message = 'Goodbye'
    print(hello.message)
