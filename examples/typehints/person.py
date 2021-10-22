#------------------------------------------------------------------------------
# Copyright (c) 2013-2017, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
#------------------------------------------------------------------------------
"""Simple class using atom and static observers.

"""
from __future__ import (division, unicode_literals, print_function,
                        absolute_import)

from atom.api import Atom, Str, Range, Bool, observe
from typing import Optional

class Person(Atom):
    """ A simple class representing a person object.

    """
    last_name: str

    first_name: str

    middle_name: Optional[str]

    age = Range(low=0)

    debug: bool = False

    @observe('age')
    def debug_print(self, change):
        """ Prints out a debug message whenever the person's age changes.

        """
        if self.debug:
            templ = "{first} {middle} {last} is {age} years old."
            s = templ.format(
                first=self.first_name, middle=self.middle_name, last=self.last_name, age=self.age,
            )

            print(s)


if __name__ == '__main__':
    john = Person(first_name='John', last_name='Doe', age=42)
    john.debug = True
    john.age = 43
