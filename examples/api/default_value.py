# --------------------------------------------------------------------------------------
# Copyright (c) 2013-2023, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
""" Demonstrate all the ways to initialize a value

1. Pass the value directly
2. Assign the default value explicitly
3. Provide the value during initialization of the object
4. Provide factory callable that returns a value
5. Use a _default_* static method

"""
import sys

from atom.api import Atom, Int, Str


def get_mother():
    return "Maude " + get_last_name()


def get_last_name():
    """Return a last name based on the system byteorder."""
    return sys.byteorder.capitalize()


class Person(Atom):
    """A simple class representing a person object."""

    first_name = Str("Bob")

    age = Int(default=40)

    address = Str()

    mother = Str(factory=get_mother)

    last_name = Str()

    def _default_last_name(self):
        return get_last_name()


if __name__ == "__main__":
    bob = Person(address="101 Main")
    print((bob.first_name, bob.last_name, bob.age))
    print(bob.mother)
