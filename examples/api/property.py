#------------------------------------------------------------------------------
# Copyright (c) 2014, Nucleic
#
# Distributed under the terms of the BSD 3-Clause License.
#
# The full license is in the file LICENSE, distributed with this software.
#------------------------------------------------------------------------------
from __future__ import print_function

from atom.api import Atom, Unicode, Property, Int


class Person(Atom):
    """ A simple class representing a person object.

    """
    first_name = Unicode()

    age = Property()
    _age = Int(40)

    def _get_age(self):
        return self._age

    def _set_age(self, age):
        if age < 100 and age > 0:
            self._age = age


if __name__ == '__main__':
    bob = Person(first_name='Bob')
    print(bob.age)
    bob.age = -10
    print(bob.age)
    bob.age = 20
    print(bob.age)
