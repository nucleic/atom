# --------------------------------------------------------------------------------------
# Copyright (c) 2018-2023, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
"""Example demonstrating the use of metadata to filter members.

"""
import sys

from atom.api import Atom, Int, Str


def get_last_name():
    """Return a last name based on the system byteorder."""
    return sys.byteorder.capitalize()


class Person(Atom):
    """A simple class representing a person object."""

    first_name = Str("Bob").tag(pref=True)

    age = Int(default=40).tag(pref=False)

    last_name = Str()

    def _default_last_name(self):
        return get_last_name()


if __name__ == "__main__":
    bob = Person()

    for name, member in bob.members().items():
        if member.metadata and "pref" in member.metadata:
            print(name, member.metadata["pref"])
