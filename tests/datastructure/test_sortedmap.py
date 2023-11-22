# --------------------------------------------------------------------------------------
# Copyright (c) 2013-2023, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
"""Test the sortedmap that acts like an ordered dictionary.

"""
import gc

import pytest

from atom.api import Atom, Value, atomref
from atom.datastructures.api import sortedmap


@pytest.fixture
def smap():
    """Sortedmap used for testing."""
    smap = sortedmap()
    smap["a"] = 1
    smap["b"] = 2
    smap["c"] = 3
    return smap


def test_sortedmap_init():
    """Test initializing a sortedmap."""
    smap = sortedmap({})
    assert smap.items() == []
    smap = sortedmap([(1, 2)])
    assert smap.items() == [(1, 2)]
    smap = sortedmap({1: 2})
    assert smap.items() == [(1, 2)]

    with pytest.raises(TypeError):
        sortedmap(1)
    with pytest.raises(TypeError) as excinfo:
        sortedmap([1])
    assert "pairs" in excinfo.exconly()


def test_traverse():
    """Test traversing on deletion."""

    class Holder(Atom):
        smap = Value()

    h = Holder()
    smap = sortedmap()

    # Create a reference cycle
    h.smap = smap
    smap[1] = h

    # Create a weakref to check that the objects involved in teh cycle are
    # collected
    ref = atomref(h)

    del smap, h
    gc.collect()

    assert not ref()


def test_contains(smap):
    """Test contains test."""
    assert "a" in smap
    assert 1 not in smap


def test_indexing(smap):
    """Test using indexing for get and set operations."""
    assert smap["a"] == 1
    smap["a"] = 2
    assert smap["a"] == 2

    with pytest.raises(KeyError):
        smap[1]
    with pytest.raises(KeyError):
        sortedmap()[1]


def test_get(smap):
    """Test the get method of sortedmap."""
    assert smap.get("a") == 1
    assert smap.get("d") is None
    assert smap.get("e", 4) == 4

    # Test bad parameters for get
    with pytest.raises(TypeError):
        smap.get()
    with pytest.raises(TypeError):
        smap.get("r", None, None)


def test_pop(smap):
    """Test the pop method of sortedmap."""
    assert smap.pop("b") == 2
    assert "b" not in smap
    assert smap.pop("b", 1) == 1
    assert smap.keys() == ["a", "c"]
    assert smap.pop("d", 1) == 1

    with pytest.raises(KeyError):
        smap.pop("b")

    # Test bad parameters for pop
    with pytest.raises(TypeError):
        smap.pop()
    with pytest.raises(TypeError):
        smap.pop(None, None, None)


def test_keys_values_items(smap):
    """Test the keys, values and items."""
    assert smap.keys() == ["a", "b", "c"]
    assert smap.values() == [1, 2, 3]
    assert smap.items() == [("a", 1), ("b", 2), ("c", 3)]


def test_iter(smap):
    """Test iterating sortedmap."""
    keys = smap.keys()
    for i, k in enumerate(smap):
        assert keys[i] == k


def test_ordering_with_inhomogeneous(smap):
    """Test the ordering of the map."""
    smap["d"] = 4
    assert list(smap.keys()) == ["a", "b", "c", "d"]

    smap["0"] = 4
    assert list(smap.keys()) == ["0", "a", "b", "c", "d"]

    smap[1] = 4
    assert list(smap.keys()) == [1, "0", "a", "b", "c", "d"]

    # Test ordering None, which is smaller than anything
    s = sortedmap()
    s[1] = 1
    s[None] = 1
    assert list(s.keys()) == [None, 1]

    s = sortedmap()
    s[None] = 1
    s[1] = 1
    assert list(s.keys()) == [None, 1]

    # Test ordering class that cannot be ordered through the usual mean
    class T:
        pass

    t1 = T()
    t2 = T()
    oT = T

    class T:
        pass

    u = T()
    s = sortedmap()
    s[t1] = 1
    s[t2] = 1
    assert list(s.keys()) == [t1, t2] if id(t1) < id(t2) else [t2, t1]
    s[u] = 1
    assert next(iter(s.keys())) is u if id(T) < id(oT) else list(s.keys())[-1] is u


def test_deleting_keys(smap):
    """Test deleting items."""
    del smap["c"]
    assert smap.keys() == ["a", "b"]

    with pytest.raises(KeyError):
        del smap[1]
    with pytest.raises(KeyError):
        del sortedmap()[1]


def test_repr(smap):
    """Test getting the repr of the map."""
    assert "sortedmap" in repr(smap)
    new_smap = eval(repr(smap))
    assert new_smap.items() == smap.items()
    # assert eval(repr(smap)) == smap


def test_copying(smap):
    """Test copying a sortedmap."""
    csmap = smap.copy()
    assert csmap is not smap
    assert csmap.keys() == smap.keys()
    assert csmap.values() == smap.values()
    assert csmap.items() == smap.items()


def test_sizeof(smap):
    """Test comuting the size."""
    smap.__sizeof__()


def test_clear(smap):
    """Test clearing a map."""
    smap.clear()
    assert not smap
