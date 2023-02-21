# --------------------------------------------------------------------------------------
# Copyright (c) 2013-2023, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
"""Test the enum member.

"""
import pytest

from atom.api import Enum


def test_enum():
    """Test manipulating an Enum member."""
    e = Enum("a", "b")
    assert e.items == ("a", "b")
    assert e.default_value_mode[1] == "a"

    e_def = e("b")
    assert e_def is not e
    assert e_def.default_value_mode[1] == "b"
    with pytest.raises(TypeError):
        e("c")

    e_add = e.added("c", "d")
    assert e_add is not e
    assert e_add.items == ("a", "b", "c", "d")

    e_rem = e.removed("a")
    assert e_rem is not e
    assert e_rem.items == ("b",)
    assert e_rem.default_value_mode[1] == "b"
    with pytest.raises(ValueError):
        e.removed("a", "b")

    with pytest.raises(ValueError):
        Enum()
