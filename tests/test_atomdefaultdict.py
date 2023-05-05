# --------------------------------------------------------------------------------------
# Copyright (c) 2018-2023, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
"""Test the typed dictionary.


"""
from collections import defaultdict

import pytest

from atom.api import Atom, DefaultDict, Instance, Int, List, atomlist, defaultatomdict


@pytest.fixture
def default_atom_dict():
    """Atom with different Dict members."""

    class DictAtom(Atom):
        untyped = DefaultDict(missing=int)
        keytyped = DefaultDict(Int(), missing=int)
        valuetyped = DefaultDict(value=Int())
        valuetyped_missing = DefaultDict(value=Int(1), missing=int)
        fullytyped = DefaultDict(Int(), Int())
        fullytyped_missing = DefaultDict(Int(), Int(1), missing=int)
        untyped_default = DefaultDict(default=defaultdict(int, {1: 1}))
        untyped_default_missing = DefaultDict(default={1: 1}, missing=int)
        keytyped_default = DefaultDict(Int(), default=defaultdict(int, {1: 1}))
        valuetyped_default = DefaultDict(value=Int(), default={1: 1})
        valuetyped_default_missing = DefaultDict(
            value=Int(1), default={1: 1}, missing=int
        )
        fullytyped_default = DefaultDict(Int(), Int(), default={1: 1})
        fullytyped_default_missing = DefaultDict(
            Int(), Int(1), default={1: 1}, missing=int
        )

    return DictAtom()


MEMBERS = [
    "untyped",
    "keytyped",
    "valuetyped",
    "valuetyped_missing",
    "fullytyped",
    "fullytyped_missing",
    "untyped_default",
    "keytyped_default",
    "valuetyped_default",
    "valuetyped_default_missing",
    "fullytyped_default",
    "fullytyped_default_missing",
]


@pytest.mark.parametrize("member", MEMBERS)
def test_instance(default_atom_dict, member):
    """Test the repr."""
    assert isinstance(getattr(default_atom_dict, member), defaultatomdict)


@pytest.mark.parametrize("member", MEMBERS)
def test_repr(default_atom_dict, member):
    """Test the repr."""
    m = getattr(default_atom_dict.__class__, member)
    d = m.default_value_mode[1]
    if not d or not isinstance(d, defaultdict):
        d = defaultdict(m.validate_mode[1][2], {i: i**2 for i in range(10)})
        setattr(default_atom_dict, member, d)
    assert repr(getattr(default_atom_dict, member)) == repr(d)


@pytest.mark.parametrize("member", MEMBERS)
def test_len(default_atom_dict, member):
    """Test the len."""
    d = getattr(default_atom_dict.__class__, member).default_value_mode[1]
    if not d:
        d = {i: i**2 for i in range(10)}
        setattr(default_atom_dict, member, d)
    assert len(getattr(default_atom_dict, member)) == len(d)


@pytest.mark.parametrize("member", MEMBERS)
def test_contains(default_atom_dict, member):
    """Test __contains__."""
    d = {i: i**2 for i in range(10)}
    setattr(default_atom_dict, member, d)
    assert 5 in getattr(default_atom_dict, member)
    del getattr(default_atom_dict, member)[5]
    assert 5 not in getattr(default_atom_dict, member)


@pytest.mark.parametrize("member", MEMBERS)
def test_keys(default_atom_dict, member):
    """Test the keys."""
    d = getattr(default_atom_dict.__class__, member).default_value_mode[1]
    if not d:
        d = {i: i**2 for i in range(10)}
        setattr(default_atom_dict, member, d)
    assert getattr(default_atom_dict, member).keys() == d.keys()


@pytest.mark.parametrize("member", MEMBERS)
def test_copy(default_atom_dict, member):
    """Test copy."""
    d = getattr(default_atom_dict.__class__, member).default_value_mode[1]
    if not d:
        d = {i: i**2 for i in range(10)}
        setattr(default_atom_dict, member, d)
    assert getattr(default_atom_dict, member).copy() == d


def test_bad_missing_callable():
    with pytest.raises(ValueError) as exc:

        class DA(Atom):
            broken_d = DefaultDict(missing=lambda: int(""))

    assert "The missing argument expect a callable taking no argument" in exc.exconly()


def test_setitem(default_atom_dict):
    """Test setting items."""
    default_atom_dict.untyped[""] = 1
    assert default_atom_dict.untyped[""] == 1

    default_atom_dict.keytyped[1] = ""
    assert default_atom_dict.keytyped[1] == ""
    with pytest.raises(TypeError):
        default_atom_dict.keytyped[""] = 1

    default_atom_dict.valuetyped[1] = 1
    assert default_atom_dict.valuetyped[1] == 1
    with pytest.raises(TypeError):
        default_atom_dict.valuetyped[""] = ""

    default_atom_dict.fullytyped[1] = 1
    assert default_atom_dict.fullytyped[1] == 1
    with pytest.raises(TypeError):
        default_atom_dict.fullytyped[""] = 1
    with pytest.raises(TypeError):
        default_atom_dict.fullytyped[1] = ""


def test_setdefault(default_atom_dict):
    """Test using setdefault."""

    assert default_atom_dict.untyped.setdefault("", 1) == 1
    assert default_atom_dict.untyped.setdefault("", 2) == 1
    assert default_atom_dict.untyped[""] == 1

    assert default_atom_dict.keytyped.setdefault(1, "") == ""
    assert default_atom_dict.keytyped[1] == ""
    with pytest.raises(TypeError):
        default_atom_dict.keytyped.setdefault("", 1)

    assert default_atom_dict.valuetyped.setdefault(1, 1) == 1
    assert default_atom_dict.valuetyped.setdefault(1, "") == 1
    assert default_atom_dict.valuetyped[1] == 1
    with pytest.raises(TypeError):
        default_atom_dict.valuetyped.setdefault(2, "")

    assert default_atom_dict.fullytyped.setdefault(1, 1) == 1
    assert default_atom_dict.fullytyped.setdefault(1, "") == 1
    assert default_atom_dict.fullytyped[1] == 1
    with pytest.raises(TypeError):
        default_atom_dict.fullytyped.setdefault("", 1)
    with pytest.raises(TypeError):
        default_atom_dict.fullytyped.setdefault(2, "")


def test_setdefault_coercion():
    class A(Atom):
        d = DefaultDict(int, List(int))

    a = A()
    content = a.d.setdefault(1, [])
    assert isinstance(content, atomlist)
    assert content is a.d[1]


def test_update(default_atom_dict):
    """Test update a dict."""
    default_atom_dict.untyped.update({"": 1})
    assert default_atom_dict.untyped[""] == 1
    default_atom_dict.untyped.update([("1", 1)])
    assert default_atom_dict.untyped["1"] == 1

    default_atom_dict.keytyped.update({1: 1})
    assert default_atom_dict.keytyped[1] == 1
    default_atom_dict.keytyped.update([(2, 1)])
    assert default_atom_dict.keytyped[1] == 1
    with pytest.raises(TypeError):
        default_atom_dict.keytyped.update({"": 1})

    default_atom_dict.valuetyped.update({1: 1})
    assert default_atom_dict.valuetyped[1] == 1
    default_atom_dict.valuetyped.update([(2, 1)])
    assert default_atom_dict.valuetyped[1] == 1
    with pytest.raises(TypeError):
        default_atom_dict.valuetyped.update({"": ""})

    default_atom_dict.fullytyped.update({1: 1})
    assert default_atom_dict.fullytyped[1] == 1
    default_atom_dict.fullytyped.update([(2, 1)])
    assert default_atom_dict.fullytyped[1] == 1
    with pytest.raises(TypeError):
        default_atom_dict.fullytyped.update({"": 1})
    with pytest.raises(TypeError):
        default_atom_dict.fullytyped.update({"": ""})


@pytest.mark.parametrize("member_name", MEMBERS)
def test_missing(default_atom_dict, member_name):
    assert getattr(default_atom_dict, member_name)[-1] == 0


def test_missing_with_trivial_instance():
    class A(Atom):
        d = DefaultDict(int, int)

    assert isinstance(A.d.validate_mode[1][1], Instance)
    a = A()
    assert a.d[1] == 0
    with pytest.raises(RuntimeError):
        A().d[1]


def test_coerced_missing():
    class A(Atom):
        d = DefaultDict(int, List(int))

    a = A()
    content = a.d[1]
    assert isinstance(content, atomlist)
    assert content is a.d[1]
