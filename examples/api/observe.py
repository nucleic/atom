# --------------------------------------------------------------------------------------
# Copyright (c) 2013-2023, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
"""Demonstration of the use of static and dynamic observers.

"""
from atom.api import Atom, ChangeDict, Range, Str, Typed, observe


class Dog(Atom):
    name = Str()


class Person(Atom):
    """A simple class representing a person object."""

    name = Str()

    age = Range(low=0)

    dog = Typed(Dog, ())

    def _observe_age(self, change: ChangeDict) -> None:
        print("Age changed: {0}".format(change["value"]))

    @observe("name")
    def any_name_i_want(self, change: ChangeDict) -> None:
        print("Name changed: {0}".format(change["value"]))

    @observe("dog.name")
    def another_random_name(self, change: ChangeDict) -> None:
        print("Dog name changed: {0}".format(change["value"]))


class Employee(Person):
    title = Str()

    @observe("title")
    def on_title_change(self, change: ChangeDict) -> None:
        super().any_name_i_want(change)


def main():
    bob = Person(name="Robert Paulson", age=40)
    bob.name = "Bobby Paulson"
    bob.age = 50
    bob.dog.name = "Scruffy"

    def watcher_func(change):
        print("Watcher func change: {0}".format(change["value"]))

    bob.observe("age", watcher_func)
    bob.age = 51
    bob.unobserve("age", watcher_func)
    bob.age = 52  # No call to watcher func


if __name__ == "__main__":
    main()
