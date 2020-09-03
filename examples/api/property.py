#------------------------------------------------------------------------------
# Copyright (c) 2013-2017, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
#------------------------------------------------------------------------------
"""Demonstration of the basics of the Property member.

"""
from __future__ import (division, unicode_literals, print_function,
                        absolute_import)

from atom.api import Atom, Str, Property, Int


class Person(Atom):
    """ A simple class representing a person object.

    """
    first_name = Str()

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
