# --------------------------------------------------------------------------------------
# Copyright (c) 2013-2023, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
"""Test the working of the Atom class and metaclass

The handling of specially named method is not tested here as it is exercised
in the test dedicated to the associated behaviors.

The methods related to member observation are tested in test_observe.py

"""
import gc
import pickle
from textwrap import dedent

import pytest

from atom.api import (
    Atom,
    Int,
    MissingMemberWarning,
    Str,
    Value,
    add_member,
    atomref,
    clone_if_needed,
    observe,
    set_default,
)


def test_init():
    """Test init."""

    class A(Atom):
        val = Int()

    a = A(val=2)
    assert a.val == 2

    with pytest.raises(TypeError):
        A(None)

    # Simply check it does not crash
    a.__sizeof__()


def test_set_default():
    """Test changing the default value of a member."""

    class Default1(Atom):
        i = Int()
        i2 = Int()

    sd = set_default(1)

    class Default2(Default1):
        i = sd
        i2 = sd

    # By setting the same default twice we should get a clone
    assert Default1.i is not Default2.i
    assert Default1().i == 0
    assert Default2().i == 1

    with pytest.raises(TypeError):

        class Check(Atom):
            a = set_default(1)


def test_multi_inheritance():
    """Test that multiple inheritance does not break the memory layout."""

    class Multi1(Atom):
        i1 = Int()

        i2 = Int()

    class Multi2(Atom):
        i3 = Int()

        i4 = Int()

    # This ensures the conflict will occur (dict lack of ordering can cause
    # unexpected mismatch)
    assert Multi1.i1.index == Multi2.i3.index or Multi1.i1.index == Multi2.i4.index
    assert Multi1.i2.index == Multi2.i3.index or Multi1.i2.index == Multi2.i4.index

    class Multi(Multi1, Multi2):
        i4 = Int(12)  # Test that conflicts do not mess up overridden members

    assert Multi().i4 == 12
    members = Multi().members()
    for m in members.values():
        for m2 in members.values():
            if m is m2:
                continue
            assert m.index != m2.index

    class Mixin(Atom):
        i5 = Int()

        i6 = Int()

        i7 = Int()

    class MultiNext(Multi, Mixin):
        i1 = Int()

    class MultiNext2(MultiNext):
        i1 = Int()

    assert sorted(m.index for m in MultiNext2.__atom_members__.values()) == list(
        range(7)
    )


def test_member_mro_in_multiple_inheritance():
    """Test that we respect the MRO for members."""

    class A(Atom):
        a = Str("a")

    class B(Atom):
        b = Str("b")

    class AB(A, B):
        pass

    class A2(A):
        a = Str("a2")

    class C(AB, A2):
        pass

    # C mro AB -> A2 -> A -> B -> Atom
    # a is not defined or altered on AB so we expect to get A2 behavior
    assert C().a == "a2"

    # When AB alters a we expect to see it
    class BB(A, B):
        a = set_default("b")

    class D(BB, A2):
        pass

    assert D().a == "b"


def test_add_member():
    class A(Atom):
        pass

    add_member(A, "a", Int())

    class B(A):
        pass

    assert "a" in B().members()


def test_add_member_overridden_member():
    class A(Atom):
        a = Int()
        b = Int()

        def _observe_b(self, change):
            pass

    class B(A):
        c = Int()

    new = Str()
    add_member(B, "b", new)

    r = B(a=1, b="abc")
    assert r.a == 1
    assert r.b == "abc"
    assert B.a.index == 0
    assert B.b.index == 1
    assert B.c.index == 2
    assert B.b.has_observer("_observe_b")


def test_cloning_members():
    """Test cloning a member when appropriate.

    Here we test assigning the same member to two names. Other cases in which
    cloning is required such as modifying a mode are tested in the tests

    """

    class CloneTest(Atom):
        a = b = Int()

    assert CloneTest.a is not CloneTest.b


def test_listing_members():
    """Test listing the members from an Atom instance."""

    class MembersTest(Atom):
        a = b = c = d = e = Int()

    assert sorted(MembersTest().members().keys()) == ["a", "b", "c", "d", "e"]


def test_getting_members():
    """Test accessing members directly."""

    class A(Atom):
        val = Int()

    assert A().get_member("val") is A.val
    assert A().get_member("") is None

    with pytest.raises(TypeError):
        A().get_member(1)


class PicklingTest(Atom):
    __slots__ = ("d",)

    a = b = c = Int()


# See also test_get_set_state
def test_pickling():
    """Test pickling an Atom instance."""
    pt = PicklingTest()
    pt.a = 2
    pt.b = 3
    pt.c = 4
    pt.d = 5

    pick = pickle.dumps(pt)
    loaded = pickle.loads(pick)
    assert isinstance(loaded, PicklingTest)
    assert loaded.a == 2
    assert loaded.b == 3
    assert loaded.c == 4
    assert loaded.d == 5


def test_freezing():
    """Test freezing an Atom instance."""

    class FreezingTest(Atom):
        a = Int()

    ft = FreezingTest()
    ft.a = 25
    ft.freeze()

    assert ft.a == 25
    with pytest.raises(AttributeError):
        ft.a = 1
    with pytest.raises(AttributeError):
        del ft.a


def test_traverse_atom():
    """Test that we can break reference cycles involving Atom object."""

    class MyAtom(Atom):
        val = Value()

    a = MyAtom()
    l1 = []
    a.val = l1
    a.val.append(a)

    ref = atomref(a)
    del a, l1
    gc.collect()

    assert not ref()


@pytest.mark.parametrize(
    "method_name",
    [
        "_default_x",
        "_validate_x",
        "_post_validate_x",
        "_post_getattr_",
        "_post_setattr_",
        "_getstate_",
        "_observe_",
    ],
)
def test_warn_on_missing(method_name):
    src = dedent(
        f"""
    from atom.api import Atom

    class A(Atom):

        def {method_name}(self):
            pass

    """
    )

    with pytest.warns(MissingMemberWarning):
        exec(src)


def test_warn_on_missing_observe():
    src = dedent(
        """
    class A(Atom):

        @observe("x")
        def f(self, change):
            pass

    """
    )

    with pytest.warns(MissingMemberWarning):
        exec(src, globals(), {"Atom": Atom, "observe": observe})


def test_enable_weakref():
    class WA(Atom, enable_weakrefs=True):
        pass

    assert "__weakref__" in WA.__slots__


def test_init_subclass():
    members = {}

    class A(Atom):
        def __init_subclass__(cls) -> None:
            super().__init_subclass__()
            nonlocal members
            members = cls.members()

    class B(A):
        a = Int()

    assert members == {"a": B.a}


def test_clone_if_needed():
    class A(Atom):
        def __init_subclass__(cls) -> None:
            for m in cls.members().values():
                clone_if_needed(cls, m)

    class B(A):
        a = Int()

    i = Int()

    class C(B):
        b = i

    assert C.a is not B.a
    assert C.b is i
