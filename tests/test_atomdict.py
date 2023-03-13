# --------------------------------------------------------------------------------------
# Copyright (c) 2018-2023, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
"""Test the typed dictionary.


"""
import pytest

from atom.api import Atom, Dict, Int, List, atomdict, atomlist


@pytest.fixture
def atom_dict():
    """Atom with different Dict members."""

    class DictAtom(Atom):
        untyped = Dict()
        keytyped = Dict(Int())
        valuetyped = Dict(value=Int())
        fullytyped = Dict(Int(), Int())
        untyped_default = Dict(default={1: 1})
        keytyped_default = Dict(Int(), default={1: 1})
        valuetyped_default = Dict(value=Int(), default={1: 1})
        fullytyped_default = Dict(Int(), Int(), default={1: 1})

    return DictAtom()


MEMBERS = [
    "untyped",
    "keytyped",
    "valuetyped",
    "fullytyped",
    "untyped_default",
    "keytyped_default",
    "valuetyped_default",
    "fullytyped_default",
]


@pytest.mark.parametrize("member", MEMBERS)
def test_instance(atom_dict, member):
    """Test the repr."""
    assert isinstance(getattr(atom_dict, member), atomdict)


@pytest.mark.parametrize("member", MEMBERS)
def test_repr(atom_dict, member):
    """Test the repr."""
    d = getattr(atom_dict.__class__, member).default_value_mode[1]
    if not d:
        d = {i: i**2 for i in range(10)}
        setattr(atom_dict, member, d)
    assert repr(getattr(atom_dict, member)) == repr(d)


@pytest.mark.parametrize("member", MEMBERS)
def test_len(atom_dict, member):
    """Test the len."""
    d = getattr(atom_dict.__class__, member).default_value_mode[1]
    if not d:
        d = {i: i**2 for i in range(10)}
        setattr(atom_dict, member, d)
    assert len(getattr(atom_dict, member)) == len(d)


@pytest.mark.parametrize("member", MEMBERS)
def test_contains(atom_dict, member):
    """Test __contains__."""
    d = {i: i**2 for i in range(10)}
    setattr(atom_dict, member, d)
    assert 5 in getattr(atom_dict, member)
    del getattr(atom_dict, member)[5]
    assert 5 not in getattr(atom_dict, member)


@pytest.mark.parametrize("member", MEMBERS)
def test_keys(atom_dict, member):
    """Test the keys."""
    d = getattr(atom_dict.__class__, member).default_value_mode[1]
    if not d:
        d = {i: i**2 for i in range(10)}
        setattr(atom_dict, member, d)
    assert getattr(atom_dict, member).keys() == d.keys()


@pytest.mark.parametrize("member", MEMBERS)
def test_copy(atom_dict, member):
    """Test copy."""
    d = getattr(atom_dict.__class__, member).default_value_mode[1]
    if not d:
        d = {i: i**2 for i in range(10)}
        setattr(atom_dict, member, d)
    assert getattr(atom_dict, member).copy() == d


def test_setitem(atom_dict):
    """Test setting items."""
    atom_dict.untyped[""] = 1
    assert atom_dict.untyped[""] == 1

    atom_dict.keytyped[1] = ""
    assert atom_dict.keytyped[1] == ""
    with pytest.raises(TypeError):
        atom_dict.keytyped[""] = 1

    atom_dict.valuetyped[1] = 1
    assert atom_dict.valuetyped[1] == 1
    with pytest.raises(TypeError):
        atom_dict.valuetyped[""] = ""

    atom_dict.fullytyped[1] = 1
    assert atom_dict.fullytyped[1] == 1
    with pytest.raises(TypeError):
        atom_dict.fullytyped[""] = 1
    with pytest.raises(TypeError):
        atom_dict.fullytyped[1] = ""


def test_setdefault(atom_dict):
    """Test using setdefault."""

    assert atom_dict.untyped.setdefault("", 1) == 1
    assert atom_dict.untyped.setdefault("", 2) == 1
    assert atom_dict.untyped[""] == 1

    assert atom_dict.keytyped.setdefault(1, "") == ""
    assert atom_dict.keytyped[1] == ""
    with pytest.raises(TypeError):
        atom_dict.keytyped.setdefault("", 1)

    assert atom_dict.valuetyped.setdefault(1, 1) == 1
    assert atom_dict.valuetyped.setdefault(1, "") == 1
    assert atom_dict.valuetyped[1] == 1
    with pytest.raises(TypeError):
        atom_dict.valuetyped.setdefault(2, "")

    assert atom_dict.fullytyped.setdefault(1, 1) == 1
    assert atom_dict.fullytyped.setdefault(1, "") == 1
    assert atom_dict.fullytyped[1] == 1
    with pytest.raises(TypeError):
        atom_dict.fullytyped.setdefault("", 1)
    with pytest.raises(TypeError):
        atom_dict.fullytyped.setdefault(2, "")


def test_setdefault_coercion():
    class A(Atom):
        d = Dict(int, List(int))

    a = A()
    content = a.d.setdefault(1, [])
    assert isinstance(content, atomlist)
    assert content is a.d[1]


def test_update(atom_dict):
    """Test update a dict."""
    atom_dict.untyped.update({"": 1})
    assert atom_dict.untyped[""] == 1
    atom_dict.untyped.update([("1", 1)])
    assert atom_dict.untyped["1"] == 1

    atom_dict.keytyped.update({1: 1})
    assert atom_dict.keytyped[1] == 1
    atom_dict.keytyped.update([(2, 1)])
    assert atom_dict.keytyped[1] == 1
    with pytest.raises(TypeError):
        atom_dict.keytyped.update({"": 1})

    atom_dict.valuetyped.update({1: 1})
    assert atom_dict.valuetyped[1] == 1
    atom_dict.valuetyped.update([(2, 1)])
    assert atom_dict.valuetyped[1] == 1
    with pytest.raises(TypeError):
        atom_dict.valuetyped.update({"": ""})

    atom_dict.fullytyped.update({1: 1})
    assert atom_dict.fullytyped[1] == 1
    atom_dict.fullytyped.update([(2, 1)])
    assert atom_dict.fullytyped[1] == 1
    with pytest.raises(TypeError):
        atom_dict.fullytyped.update({"": 1})
    with pytest.raises(TypeError):
        atom_dict.fullytyped.update({"": ""})
