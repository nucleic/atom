# --------------------------------------------------------------------------------------
# Copyright (c) 2013-2023, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
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

from atom.api import (
    Atom,
    DefaultValue,
    Dict,
    Event,
    ForwardInstance,
    ForwardSubclass,
    ForwardTyped,
    GetAttr,
    GetState,
    Int,
    List,
    PostGetAttr,
    PostSetAttr,
    PostValidate,
    Set,
    SetAttr,
    Tuple,
    Validate,
    Value,
    observe,
)
from atom.catom import DelAttr


def test_name_managing_name():
    """Test getting/setting the name of a Member."""

    class NameTest(Atom):
        v = Value()

        t = Tuple(Int())

        li = List(Int())

        d = Dict(Int(), Int())

        e = Event(Int())

    assert NameTest.v.name == "v"
    NameTest.v.set_name("v2")
    assert NameTest.v.name == "v2"

    assert NameTest.t.name == "t"
    assert NameTest.t.item.name == "t|item"

    assert NameTest.li.name == "li"
    assert NameTest.li.item.name == "li|item"

    assert NameTest.d.name == "d"
    key, value = NameTest.d.validate_mode[1]
    assert key.name == "d|key"
    assert value.name == "d|value"

    assert NameTest.e.name == "e"
    assert NameTest.e.validate_mode[1].name == "e"

    with pytest.raises(TypeError) as excinfo:
        NameTest.v.set_name(1)
    assert "str" in excinfo.exconly()


def test_managing_slot_index():
    """Test getting and setting the index of a Member."""

    class IndexTest(Atom):
        v1 = Value()
        v2 = Value()

        t = Tuple(Int())

        li = List(Int())

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

    assert IndexTest.t.item.index == IndexTest.t.index
    IndexTest.t.set_index(99)
    assert IndexTest.t.item.index == IndexTest.t.index

    assert IndexTest.li.item.index == IndexTest.li.index
    IndexTest.li.set_index(99)
    assert IndexTest.li.item.index == IndexTest.li.index

    key, value = IndexTest.d.validate_mode[1]
    assert key.index == IndexTest.d.index
    assert value.index == IndexTest.d.index
    IndexTest.d.set_index(99)
    assert key.index == IndexTest.d.index
    assert value.index == IndexTest.d.index

    assert IndexTest.e.validate_mode[1].index == IndexTest.e.index

    with pytest.raises(TypeError) as excinfo:
        IndexTest.v1.set_index("")
    assert "int" in excinfo.exconly()


def test_metadata_handling():
    """Test writing and accessing the metadata of a Member."""
    assert Value().metadata is None

    class MetadataTest(Atom):
        m = Value().tag(pref=True)

    mt = MetadataTest()
    m = mt.get_member("m")
    assert m.metadata == {"pref": True}
    m.metadata = {"a": 1, "b": 2}
    assert m.metadata == {"a": 1, "b": 2}

    m.metadata = None
    assert m.metadata is None

    with pytest.raises(TypeError) as excinfo:
        m.metadata = 1
    assert "dict or None" in excinfo.exconly()

    with pytest.raises(TypeError) as excinfo:
        m.tag(1)
    assert "tag()" in excinfo.exconly()

    with pytest.raises(TypeError) as excinfo:
        m.tag()
    assert "tag()" in excinfo.exconly()


def test_direct_slot_access():
    """Test accessing a slot directly."""

    class SlotTest(Atom):
        v = Value()

    st = SlotTest()
    assert SlotTest.v.get_slot(st) is None
    SlotTest.v.set_slot(st, 1)
    assert SlotTest.v.get_slot(st) == 1
    SlotTest.v.del_slot(st)
    assert SlotTest.v.get_slot(st) is None

    # Test type validation
    with pytest.raises(TypeError) as excinfo:
        SlotTest.v.get_slot(None)
    assert "CAtom" in excinfo.exconly()

    with pytest.raises(TypeError) as excinfo:
        SlotTest.v.set_slot()
    assert "2 arguments" in excinfo.exconly()

    with pytest.raises(TypeError) as excinfo:
        SlotTest.v.set_slot(None, 1)
    assert "CAtom" in excinfo.exconly()

    with pytest.raises(TypeError) as excinfo:
        SlotTest.v.del_slot(None)
    assert "CAtom" in excinfo.exconly()

    # Test index validation
    SlotTest.v.set_index(SlotTest.v.index + 1)
    with pytest.raises(AttributeError):
        SlotTest.v.get_slot(st)
    with pytest.raises(AttributeError):
        SlotTest.v.set_slot(st, 1)
    with pytest.raises(AttributeError):
        SlotTest.v.del_slot(st)


def test_class_validation():
    """Test validating the type of class in the descriptor."""

    class FalseAtom(object):
        v = Value()

    fa = FalseAtom()

    with pytest.raises(TypeError) as excinfo:
        fa.v
    assert "CAtom" in excinfo.exconly()

    with pytest.raises(TypeError) as excinfo:
        fa.v = 1
    assert "CAtom" in excinfo.exconly()


@pytest.mark.parametrize(
    "method, arg_number",
    [
        ("do_getattr", 1),
        ("do_setattr", 2),
        ("do_delattr", 1),
        ("do_post_getattr", 2),
        ("do_post_setattr", 3),
        ("do_default_value", 1),
        ("do_validate", 3),
        ("do_post_validate", 3),
        ("do_full_validate", 3),
        ("do_should_getstate", 1),
    ],
)
def test_handling_arg_issue_in_do_methods(method, arg_number):
    """Test handling bad args in do methods."""
    m = Value()
    if arg_number > 1:
        with pytest.raises(TypeError) as excinfo:
            getattr(m, method)()
        assert str(arg_number) in excinfo.exconly()

    with pytest.raises(TypeError) as excinfo:
        getattr(m, method)(*([None] * arg_number))


def test_member_cloning():
    """Test cloning members."""

    class Spy(object):
        def __call__(self, *args):
            pass

    spy = Spy()

    class CloneTest(Atom):
        v = Value().tag(test=True)

        @observe("v")
        def react(self, change):
            pass

    assert CloneTest.v.static_observers()
    CloneTest.v.set_getattr_mode(GetAttr.CallObject_Object, spy)
    CloneTest.v.set_setattr_mode(SetAttr.CallObject_ObjectValue, spy)
    CloneTest.v.set_delattr_mode(DelAttr.Signal, None)
    CloneTest.v.set_default_value_mode(DefaultValue.CallObject_Object, spy)
    CloneTest.v.set_validate_mode(Validate.Int, None)
    CloneTest.v.set_post_getattr_mode(PostGetAttr.ObjectMethod_NameValue, "a")
    CloneTest.v.set_post_setattr_mode(PostSetAttr.ObjectMethod_NameOldNew, "b")
    CloneTest.v.set_post_validate_mode(PostValidate.ObjectMethod_NameOldNew, "c")
    CloneTest.v.set_getstate_mode(GetState.ObjectMethod_Name, "s")
    cv = CloneTest.v.clone()
    for attr in (
        "name",
        "index",
        "metadata",
        "getattr_mode",
        "setattr_mode",
        "delattr_mode",
        "default_value_mode",
        "validate_mode",
        "post_getattr_mode",
        "post_setattr_mode",
        "post_validate_mode",
        "getstate_mode",
    ):
        assert getattr(cv, attr) == getattr(CloneTest.v, attr)

    assert cv.static_observers() == CloneTest.v.static_observers()


@pytest.mark.parametrize(
    "untyped, typed",
    [
        (List(), List(int)),
        (Tuple(), Tuple(int)),
        (Dict(), Dict(int, int)),
        (Set(), Set(int)),
    ],
)
def test_cloning_containers_member(untyped, typed):
    """Check that cloning a list does clone the validation item is present."""
    if not isinstance(untyped, Dict):
        assert untyped.clone().item is None

    typed.set_index(5)
    cl2 = typed.clone()
    assert cl2.index == typed.index
    validators = [typed.item] if hasattr(typed, "item") else typed.validate_mode[1]
    c_validators = [cl2.item] if hasattr(typed, "item") else cl2.validate_mode[1]
    for v, cv in zip(validators, c_validators):
        assert cv is not v
        assert isinstance(cv, type(v))


# XXX should the kwargs be copied rather than simply re-assigned
@pytest.mark.parametrize(
    "member, cloned_attributes",
    [
        (ForwardSubclass(lambda: object), ["resolve"]),
        (ForwardTyped(lambda: object, (1,), {"a": 1}), ["resolve", "args", "kwargs"]),
        (
            ForwardInstance(lambda: object, (1,), {"a": 1}),
            ["resolve", "args", "kwargs"],
        ),
    ],
)
def test_cloning_forward(member, cloned_attributes):
    """Test that subclasses of Member are properly cloned."""
    member.set_index(5)
    clone = member.clone()
    assert clone.index == member.index
    for attr in cloned_attributes:
        assert getattr(clone, attr) is getattr(member, attr)


@pytest.mark.parametrize(
    "ForwardedMember, optional",
    (
        (ForwardInstance, True),
        (ForwardTyped, False),
    ),
)
def test_cloned_forward_validator(ForwardedMember, optional):
    """Test a cloned forwarded member"""

    class AbtractItem(Atom):
        view = ForwardedMember(lambda: View, optional=optional)

    class Item(AbtractItem):
        def _default_view(self):
            return View()

    class View(Atom):
        pass

    Item().view  # Test validate
    assert Item.view.optional == optional
