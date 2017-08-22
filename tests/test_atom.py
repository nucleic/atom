#------------------------------------------------------------------------------
# Copyright (c) 2013-2017, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
"""Test the working of the Atom class and metaclass

The handling of specially named method is not tested here as it is exercised
in the test dedicated to the associated behaviors.

The methods related to member observation are tested in test_observe.py

"""
import pickle

import pytest
from atom.api import Atom, Int, set_default


def test_set_default():
    """Test changing the default value of a member.

    """
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
    """Test that multiple inheritance does not break the memory layout.

    """
    class Multi1(Atom):

        i1 = Int()

        i2 = Int()

    class Multi2(Atom):

        i3 = Int()

        i4 = Int()

    # This ensures the conflict will occur
    assert Multi1.i1.index == Multi2.i3.index
    assert Multi1.i2.index == Multi2.i4.index

    class Multi(Multi1, Multi2):

        i4 = Int(12)  # Test that conflicts do not mess up overridden members

    assert Multi().i4 == 12
    members = Multi().members()
    for m in members.values():
        for m2 in members.values():
            if m is m2:
                continue
            assert m.index != m2.index


def test_cloning_members():
    """Test cloning a member when appropriate.

    Here we test assigning the same member to two names. Other cases in which
    cloning is required such as modifying a mode are tested in the tests

    """
    class CloneTest(Atom):
        a = b = Int()

    assert CloneTest.a is not CloneTest.b


def test_listing_members():
    """Test listing the members from an Atom instance.

    """
    class MembersTest(Atom):

        a = b = c = d = e = Int()

    assert sorted(MembersTest().members().keys()) == ['a', 'b', 'c', 'd', 'e']


class PicklingTest(Atom):

    __slots__ = ('d',)

    a = b = c = Int()


def test_pickling():
    """Test pickling an Atom instance.

    """
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
    """Test freezing an Atom instance.

    """
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
