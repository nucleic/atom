# --------------------------------------------------------------------------------------
# Copyright (c) 2022, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
"""Demonstration of the interaction between static and dynamic type validation."""
from typing import List, Optional

from atom.api import Atom, Int


class MyAtom(Atom):
    """Simple atom typing example."""

    s: str = "Hello"
    lst: List[int] = [1, 2, 3]  # On Python >= 3.9 list[int] can be used
    num: Optional[float]
    n = Int()


my_atom = MyAtom()
assert my_atom.n == 0

# Optional values have a default value of None if no default is specified
assert my_atom.num is None

# Mutable default values for lists and dicts are OK
assert my_atom.lst == [1, 2, 3]

# The following statements will fail static type checking and
# Atom will raise runtime TypeError exceptions.
# (the type ignore comment allow CI to pass)
try:
    my_atom.n = "Not an integer"  # type: ignore
except TypeError as e:
    print(f"Invalid value for member 'n' of {my_atom}.\n", e)

try:
    my_atom.s = 5  # type: ignore
except TypeError as e:
    print(f"Invalid value for member 's' of {my_atom}.\n", e)
