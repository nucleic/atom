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
import pytest
from atom.api import (Atom, Value, Int, List, Dict, Event, Instance,
                      ForwardInstance, ForwardSubclass, ForwardTyped,
                      GetAttr, SetAttr, DefaultValue, Validate, PostGetAttr,
                      PostSetAttr, PostValidate, observe)
from atom.catom import DelAttr


# XXX why does tuple do not work in the same way
def test_name_managing_name():
    """Test getting/setting the name of a Member.

    """
    class NameTest(Atom):

        v = Value()

        l = List(Int())

        d = Dict(Int(), Int())

        e = Event(Int())

    assert NameTest.v.name == 'v'
    NameTest.v.set_name('v2')
    assert NameTest.v.name == 'v2'

    assert NameTest.l.name == 'l'
    assert NameTest.l.item.name == 'l|item'

    assert NameTest.d.name == 'd'
    key, value = NameTest.d.validate_mode[1]
    assert key.name == 'd|key'
    assert value.name == 'd|value'

    assert NameTest.e.name == 'e'
    assert NameTest.e.validate_mode[1].name == 'e'


def test_managing_slot_index():
    """Test getting and setting the index of a Member.

    """
    class IndexTest(Atom):

        v1 = Value()
        v2 = Value()

        l = List(Int())

        d = Dict(Int(), Int())

        e = Event(Int())

    it = IndexTest()
    it.v1 = 1
    it.v2 = 2
    id1 = IndexTest.v1.index
    id2 = IndexTest.v2.index
    IndexTest.v1.set_index(id2)
    IndexTest.v2.set_index(id1)
    assert it.v1 == 2
    assert it.v2 == 1

    assert IndexTest.l.item.index == IndexTest.l.index

    key, value = IndexTest.d.validate_mode[1]
    assert key.index == IndexTest.d.index
    assert value.index == IndexTest.d.index

    assert IndexTest.e.validate_mode[1].index == IndexTest.e.index


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
        SlotTest.v.del_slot(None)

    # Test index validation
    SlotTest.v.set_index(SlotTest.v.index + 1)
    with pytest.raises(AttributeError):
        SlotTest.v.get_slot(st)
    with pytest.raises(AttributeError):
        SlotTest.v.set_slot(st, 1)
    with pytest.raises(AttributeError):
        SlotTest.v.del_slot(st)


def test_member_cloning():
    """Test cloning members.

    """
    class Spy(object):

        def __call__(self, *args):
            pass

    spy = Spy()

    class CloneTest(Atom):
        v = Value()

        @observe('v')
        def react(self, change):
            pass

    assert CloneTest.v.static_observers()
    CloneTest.v.set_getattr_mode(GetAttr.CallObject_Object, spy)
    CloneTest.v.set_setattr_mode(SetAttr.CallObject_ObjectValue, spy)
    CloneTest.v.set_delattr_mode(DelAttr.Signal, None)
    CloneTest.v.set_default_value_mode(DefaultValue.CallObject_Object, spy)
    CloneTest.v.set_validate_mode(Validate.Int, None)
    CloneTest.v.set_post_getattr_mode(PostGetAttr.ObjectMethod_NameValue, 'a')
    CloneTest.v.set_post_setattr_mode(PostSetAttr.ObjectMethod_NameOldNew, 'b')
    CloneTest.v.set_post_validate_mode(PostValidate.ObjectMethod_NameOldNew,
                                       'c')
    cv = CloneTest.v.clone()
    for attr in ('name', 'index',
                 'getattr_mode', 'setattr_mode', 'delattr_mode',
                 'default_value_mode', 'validate_mode',
                 'post_getattr_mode', 'post_setattr_mode',
                 'post_validate_mode'):
        assert getattr(cv, attr) == getattr(CloneTest.v, attr)

    assert cv.static_observers() == CloneTest.v.static_observers()


# XXX why is only List cloning the item and not Tuple and Dict ?
def test_cloning_list():
    """Check that cloning a list does cllone the validation item is present.

    """
    l1 = List()
    assert l1.clone().item is None

    v = Instance(int)
    l2 = List(v)
    l2.set_index(5)
    cl2 = l2.clone()
    assert cl2.index == l2.index
    assert cl2.item is not v
    assert isinstance(cl2.item, type(v))
    assert cl2.item.validate_mode[1] == l2.item.validate_mode[1]


# XXX should the kwargs be copied rather than simply re-assigned
@pytest.mark.parametrize("member, cloned_attributes",
                         [(ForwardSubclass(lambda: object), ['resolve']),
                          (ForwardTyped(lambda: object, (1,), {'a': 1}),
                           ['resolve', 'args', 'kwargs']),
                          (ForwardInstance(lambda: object, (1,), {'a': 1}),
                           ['resolve', 'args', 'kwargs'])])
def test_cloning_forward(member, cloned_attributes):
    """Test that subclasses of Member are properly cloned.

    """
    member.set_index(5)
    clone = member.clone()
    assert clone.index == member.index
    for attr in cloned_attributes:
        assert getattr(clone, attr) is getattr(member, attr)
