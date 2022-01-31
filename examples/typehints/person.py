# ------------------------------------------------------------------------------
# Copyright (c) 2013-2022, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# ------------------------------------------------------------------------------
"""Person tutorial using type hints

"""

from typing import Optional

from atom.api import Atom
from atom.api import Range
from atom.api import observe


class Person(Atom):
    """ A simple class representing a person object."""

    last_name: str

    first_name: str

    middle_name: Optional[str] = None

    age = Range(low=0)

    debug: bool = False

    @observe("age")
    def debug_print(self, change):
        """ Prints out a debug message whenever the person's age changes.
        """
        if self.debug:
            templ = "{first} {middle} {last} is {age} years old."
            s = templ.format(
                first=self.first_name,
                middle=self.middle_name,
                last=self.last_name,
                age=self.age,
            )

            print(s)


if __name__ == '__main__':
    john = Person(first_name='John', last_name='Doe', age=42)
    john.debug = True
    john.age = 43
