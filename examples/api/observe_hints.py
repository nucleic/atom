# --------------------------------------------------------------------------------------
# Copyright (c) 2022, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
"""Demonstration of the use of static and dynamic observers."""
from typing import Optional

from atom.api import Atom, ChangeDict, observe


class Dog(Atom):
    name: str


class Person(Atom):
    """A simple class representing a person object."""

    name: str

    age: int

    dog: Optional[Dog]

    def _observe_age(self, change: ChangeDict) -> None:
        print("Age changed: {0}".format(change["value"]))

    @observe("name")
    def any_name_i_want(self, change: ChangeDict) -> None:
        print("Name changed: {0}".format(change["value"]))

    @observe("dog.name")
    def another_random_name(self, change: ChangeDict) -> None:
        print("Dog name changed: {0}".format(change["value"]))


def main() -> None:
    bob = Person(name="Robert Paulson", age=40)
    bob.name = "Bobby Paulson"
    bob.age = 50
    bob.dog = Dog(name="Scruffy")

    def watcher_func(change: ChangeDict) -> None:
        print("Watcher func change: {0}".format(change["value"]))

    bob.observe("age", watcher_func)
    bob.age = 51
    bob.unobserve("age", watcher_func)
    bob.age = 52  # No call to watcher func


if __name__ == "__main__":
    main()
