#------------------------------------------------------------------------------
# Copyright (c) 2013-2017, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
"""Test the Member class.

Attributes:
    - name
    - metadata
    - index
    # Tested in behavior related tests
    - (post_)getattr/(post_)setattr/delattr/default_value/(post_)validate_mode

Methods:
    - set_name (Delegator see test_delegator.py, Dict, Event, List)
    - set_index (Delegator see test_delegator.py, Dict, Event, List)
    - get_slot
    - set_slot
    - del_slot
    - clone (Member, Delegator, Instance, List, Subclass, Typed)
    - tag
    # Tested in test_observe.py
    - has_observers
    - has_observer
    - copy_static_observers
    - static_observers (Delegator see test_delegator.py)
    - add_static_oberserver (Delegator see test_delegator.py)
    - remove_static_observer (Delegator see test_delegator.py)
    - notify
    # Tested in behavior related test
    - do_(post_)getattr/(post_)setattr/delattr/default_value/(post_)validate
    - do_full_validate
    - set_getattr/setattr/delattr/default_value/validate_mode
    - set_post_getattr/setattr/validate_mode


"""
# XXX write tests
import pytest
from atom.api import (Atom, Value, ForwardInstance, List, ForwardSubclass,
                      ForwardTyped)


def test_name_managing_name():
    """Test getting/setting the name of a Member.

    """
    class NameTest(Atom):

        v = Value()

    assert NameTest.v.name == 'v'
    NameTest.v.set_name('v2')
    assert NameTest.v.name == 'v2'


def test_managing_slot_index():
    """Test getting and setting the index of a Member.

    """
    class IndexTest(Atom):

        v1 = Value()
        v2 = Value()

    it = IndexTest()
    it.v1 = 1
    it.v2 = 2
    id1 = IndexTest.v1.index
    id2 = IndexTest.v2.index
    IndexTest.v1.set_index(id2)
    IndexTest.v2.set_index(id1)
    assert it.v1 == 2
    assert it.v2 == 1


def test_metadata_handling():
    """Test writing and accessing the metadata of a Member.

    """
    class MetadataTest(Atom):

        m = Value().tag(pref=True)

    mt = MetadataTest()
    m = mt.get_member('m')
    assert m.metadata == {'pref': True}
    m.metadata = dict(a=1, b=2)
    assert m.metadata == dict(a=1, b=2)


def test_direct_slot_access():
    """Test accessing a slot directly.

    """
    class SlotTest(Atom):

        v = Value()

    st = SlotTest()
    assert SlotTest.v.get_slot(st) is None
    SlotTest.v.set_slot(st, 1)
    assert SlotTest.v.get_slot(st) == 1
    SlotTest.v.del_slot(st)
    assert SlotTest.v.get_slot(st) is None

    # Test type validation
    with pytest.raises(TypeError):
        SlotTest.v.get_slot(None)
    with pytest.raises(TypeError):
        SlotTest.v.set_slot(None, 1)
    with pytest.raises(TypeError):
        SlotTest.v.det_slot(None)

    # Test index validation
    SlotTest.v.set_index(SlotTest.v.index + 1)
    with pytest.raises(AttributeError):
        SlotTest.v.get_slot(st)
    with pytest.raises(AttributeError):
        SlotTest.v.set_slot(st, 1)
    with pytest.raises(AttributeError):
        SlotTest.v.det_slot(st)


def test_member_cloning():
    """Test cloning members.

    """
    # Need to copy all modes, contexts (imply set all modes), index, metadata,
    # and static observers
    # Forward : resolve, args, kwargs
    # List : clone internal member
    pass
