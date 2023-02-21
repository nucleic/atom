# --------------------------------------------------------------------------------------
# Copyright (c) 2013-2023, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
"""Demonstration of the basics of the Property member.

"""
from atom.api import Atom, Int, Property, Str


class Person(Atom):
    """A simple class representing a person object."""

    first_name = Str()

    # Static type checker cannot infer from the magic method that the property
    # is read/write so a type annotation helps.
    age: "Property[int, int]" = Property()  # type: ignore
    _age = Int(40)

    def _get_age(self) -> int:
        return self._age

    def _set_age(self, age: int) -> None:
        if age < 100 and age > 0:
            self._age = age


if __name__ == "__main__":
    bob = Person(first_name="Bob")
    print(bob.age)
    bob.age = -10
    print(bob.age)
    bob.age = 20
    print(bob.age)
