# --------------------------------------------------------------------------------------
# Copyright (c) 2019-2023, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
"""Test the typed dictionary.

"""
from operator import iand, ior, isub, ixor

import pytest

from atom.api import Atom, Int, Set, atomset


@pytest.fixture
def atom_set():
    """Atom with different Set members."""

    class SetAtom(Atom):
        untyped = Set()
        typed = Set(Int())
        untyped_default = Set(default={1})
        typed_default = Set(Int(), default={1})

    return SetAtom()


MEMBERS = ["untyped", "typed", "untyped_default", "typed_default"]


@pytest.mark.parametrize("member", MEMBERS)
def test_instance(atom_set, member):
    """Test the repr."""
    assert isinstance(getattr(atom_set, member), atomset)


@pytest.mark.parametrize("member", MEMBERS)
def test_repr(atom_set, member):
    """Test the repr."""
    s = getattr(atom_set.__class__, member).default_value_mode[1]
    if not s:
        s = set(range(10))
        setattr(atom_set, member, s)
    assert repr(s) in repr(getattr(atom_set, member))


@pytest.mark.parametrize("member", MEMBERS)
def test_len(atom_set, member):
    """Test the len."""
    s = getattr(atom_set.__class__, member).default_value_mode[1]
    if not s:
        s = set(range(10))
        setattr(atom_set, member, s)
    assert len(getattr(atom_set, member)) == len(s)


@pytest.mark.parametrize("member", MEMBERS)
def test_contains(atom_set, member):
    """Test __contains__."""
    s = set(range(10))
    setattr(atom_set, member, s)
    assert 5 in getattr(atom_set, member)


@pytest.mark.parametrize("member", MEMBERS)
def test_add(atom_set, member):
    """Test adding an element to a set."""
    s = getattr(atom_set.__class__, member).default_value_mode[1] or set()
    a_set = getattr(atom_set, member)
    s.add(2)
    a_set.add(2)
    assert a_set == s
    if member.startswith("typed"):
        with pytest.raises(TypeError):
            a_set.add("")


@pytest.mark.parametrize(
    "member, op, valid, result, invalid",
    [
        ("untyped", "update", {1, 3}, {1, 2, 3}, None),
        ("typed", "update", {1, 3}, {1, 2, 3}, {""}),
        ("untyped", "difference_update", {1, 3}, {2}, None),
        ("typed", "difference_update", {1, 3}, {2}, {""}),
        ("untyped", "intersection_update", {1, 3}, {1}, None),
        ("typed", "intersection_update", {1, 3}, {1}, {""}),
        ("untyped", "symmetric_difference_update", {1, 3}, {2, 3}, None),
        ("typed", "symmetric_difference_update", {1, 3}, {2, 3}, {""}),
    ],
)
def test_update_methods(atom_set, member, op, valid, result, invalid):
    """Test the different update method of a set."""
    setattr(atom_set, member, {1, 2})
    getattr(getattr(atom_set, member), op)(valid)
    assert getattr(atom_set, member) == result
    if invalid is not None:
        with pytest.raises(TypeError):
            getattr(getattr(atom_set, member), op)(invalid)


@pytest.mark.parametrize(
    "member, op, valid, result, invalid",
    [
        ("untyped", ior, {1, 3}, {1, 2, 3}, None),
        ("typed", ior, {1, 3}, {1, 2, 3}, {""}),
        ("untyped", isub, {1, 3}, {2}, None),
        ("typed", isub, {1, 3}, {2}, {""}),
        ("untyped", iand, {1, 3}, {1}, None),
        ("typed", iand, {1, 3}, {1}, {""}),
        ("untyped", ixor, {1, 3}, {2, 3}, None),
        ("typed", ixor, {1, 3}, {2, 3}, {""}),
    ],
)
def test_operations(atom_set, member, op, valid, result, invalid):
    """Test the different update method of a set."""
    a = {1, 2}
    op(a, valid)
    setattr(atom_set, member, {1, 2})
    print(a, getattr(atom_set, member), valid)
    op(getattr(atom_set, member), valid)
    assert getattr(atom_set, member) == result
    if invalid is not None:
        with pytest.raises(TypeError):
            op(getattr(atom_set, member), invalid)
