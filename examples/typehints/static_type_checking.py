# --------------------------------------------------------------------------------------
# Copyright (c) 2022, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
"""Demonstration of the interaction between static and dynamic type validation."""
from typing import Optional

from atom.api import Atom, Int


class MyAtom(Atom):
    """Simple atom typing example."""

    s: str = "Hello"
    lst: list[int] = [1, 2, 3]
    num: Optional[float]
    n = Int()


my_atom = MyAtom()
assert my_atom.n == 0

# Optional values have a default value of None if no default is specified
assert my_atom.num is None

# Mutable default values for lists and dicts are OK
assert my_atom.lst == [1, 2, 3]

# The following statements will fail static type checking and Atom will raise runtime TypeError exceptions.
my_atom.n = "Not an integer"
my_atom.s = 5
