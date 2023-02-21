# --------------------------------------------------------------------------------------
# Copyright (c) 2013-2023, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
"""Test the set behaviors

    no_op_handler: do nothing on setting
    slot_handler: standard one tested through other tests (post_set, ...)
    constant_handler: prevent to set a value
    read_only_handler: allow a single set
    event_handler: not tested here (see test_observe.py)
    signal_handler: siganls are not settable
    delegate_handler: not tested here (see test_delegate.py)
    property_handler: not tested here (see test_property.py)
    call_object_object_value_handler: use a custom function
    call_object_object_name_value_handler: use a custom function
    object_method_value_handler: use an object method
    object_method_name_value_handler: use an object method
    member_method_object_value_handler: method defined on a Member subclass

"""
import pytest

from atom.api import Atom, Constant, Int, ReadOnly, SetAttr, Signal


@pytest.mark.parametrize(
    "member, mode", [(Signal(), "Signal"), (Constant(1), "Constant")]
)
def test_unsettable(member, mode):
    """Test that unsettable members do raise the proper error."""

    class Unsettable(Atom):
        m = member

    u = Unsettable()
    assert u.get_member("m").setattr_mode[0] == getattr(SetAttr, mode)
    with pytest.raises(TypeError) as excinfo:
        u.m = 1
    assert mode.lower() in excinfo.exconly()


@pytest.mark.parametrize("member, mode", [(Int(), "Slot"), (ReadOnly(), "ReadOnly")])
def test_wrong_index_value(member, mode):
    """Test handling wrong index

    This should never happen save if the user manipulate the index.

    """

    class Unsettable(Atom):
        m = member

    Unsettable.m.set_index(100)
    u = Unsettable()
    assert u.get_member("m").setattr_mode[0] == getattr(SetAttr, mode)
    with pytest.raises(AttributeError) as excinfo:
        u.m = 1
    assert "'m'" in excinfo.exconly()


def test_read_only_behavior():
    """Test the behavior of read only member."""

    class ReadOnlyTest(Atom):
        r = ReadOnly()

    rt = ReadOnlyTest()
    rt.r = 1
    assert rt.r == 1
    with pytest.raises(TypeError) as excinfo:
        rt.r = 2
    assert "read only" in excinfo.exconly()


def test_no_op():
    """Test the no-op behavior."""

    class Unsettable(Atom):
        m = Constant("1")
        m.set_setattr_mode(SetAttr.NoOp, None)

    u = Unsettable()
    u.m = None
    assert u.m == "1"


def co_ov_factory():
    """Factory for the CallObject_ObjectValue behavior."""

    def custom_set(obj, value):
        obj.get_member("mi").set_slot(obj, value % 2)

    class SetTest(Atom):
        mi = Int()
        mi.set_setattr_mode(SetAttr.CallObject_ObjectValue, custom_set)

    return SetTest


def co_onv_factory():
    """Factory for the CallObject_ObjectNameValue behavior."""

    def custom_set(obj, name, value):
        obj.get_member(name).set_slot(obj, value % 2)

    class SetTest(Atom):
        mi = Int()
        mi.set_setattr_mode(SetAttr.CallObject_ObjectNameValue, custom_set)

    return SetTest


def om_v_factory():
    """Factory for the ObjectMethod_Value behavior."""

    class SetTest(Atom):
        mi = Int()
        mi.set_setattr_mode(SetAttr.ObjectMethod_Value, "custom_set")

        def custom_set(self, value):
            self.get_member("mi").set_slot(self, value % 2)

    return SetTest


def om_nv_factory():
    """Factory for the ObjecMethod_NameValue behavior."""

    class SetTest(Atom):
        mi = Int()
        mi.set_setattr_mode(SetAttr.ObjectMethod_NameValue, "custom_set")

        def custom_set(self, name, value):
            self.get_member(name).set_slot(self, value % 2)

    return SetTest


def mm_ov_factory():
    """Factory for the MemberMethod_ObjectValue behavior."""

    class ModuloInt(Int):
        def __init__(self):
            super(ModuloInt, self).__init__()
            mode = SetAttr.MemberMethod_ObjectValue
            self.set_setattr_mode(mode, "set")

        def set(self, obj, value):
            self.set_slot(obj, value % 2)

    class SetTest(Atom):
        mi = ModuloInt()

    return SetTest


@pytest.mark.parametrize(
    "mode, factory",
    [
        ("CallObject_ObjectValue", co_ov_factory),
        ("CallObject_ObjectNameValue", co_onv_factory),
        ("ObjectMethod_Value", om_v_factory),
        ("ObjectMethod_NameValue", om_nv_factory),
        ("MemberMethod_ObjectValue", mm_ov_factory),
    ],
)
def test_member_set_behaviors(mode, factory):
    """Test defining set in a Member subclass"""
    pvt = factory()()
    mi = pvt.get_member("mi")
    assert mi.setattr_mode[0] == getattr(SetAttr, mode)
    pvt.mi = 2
    assert pvt.mi == 0
    pvt.mi = 3
    assert pvt.mi == 1

    mi.do_setattr(pvt, 2)
    assert pvt.mi == 0
    mi.do_setattr(pvt, 3)
    assert pvt.mi == 1


@pytest.mark.parametrize(
    "mode, msg",
    [
        ("CallObject_ObjectValue", "callable"),
        ("CallObject_ObjectNameValue", "callable"),
        ("ObjectMethod_Value", "str"),
        ("ObjectMethod_NameValue", "str"),
        ("MemberMethod_ObjectValue", "str"),
    ],
)
def test_member_set_behaviors_wrong_args(mode, msg):
    """Test handling bad arguments to set_setattr_mode"""
    m = Int()
    with pytest.raises(TypeError) as excinfo:
        m.set_setattr_mode(getattr(SetAttr, mode), 1)
    assert msg in excinfo.exconly()
