# --------------------------------------------------------------------------------------
# Copyright (c) 2013-2023, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
"""Test the get behaviors

    no_op_handler: : tested here
    slot_handler: standard one tested through other tests (post_get, ...)
    event_handler: not tested here (see test_observe.py and
                                    test_event_binder.py)
    signal_handler: not tested here (see test_observe.py)
    delegate_handler: not tested here (see test_delegate.py)
    property_handler: not tested here (see test_property.py)
    cached_property_handler: not tested here (see test_property.py)
    call_object_object_handler: tested here
    call_object_object_name_handler: tested here
    object_method_handler: tested here
    object_method_name_handler: tested here
    member_method_object_handler: method defined on a Member subclass

"""
import pytest

from atom.api import Atom, GetAttr, Int, Value


def test_using_no_op_handler():
    """Test using the no_op handler."""
    v = Value()
    v.set_getattr_mode(GetAttr.NoOp, None)

    class CustomGetAtom(Atom):
        val = v

    a = CustomGetAtom()
    assert a.val is None
    a.val = 1
    assert a.val is None


def test_using_call_object_object_mode():
    """Test using call_object_object mode."""

    def getter(object):
        object.count += 1
        return object.count

    m = Int()
    m.set_getattr_mode(GetAttr.CallObject_Object, getter)

    class CustomGetAtom(Atom):
        val = m
        count = Int()

    a = CustomGetAtom()
    assert a.val == 1
    assert a.val == 2

    with pytest.raises(TypeError):
        m.set_getattr_mode(GetAttr.CallObject_Object, 1)


def test_using_call_object_object_name_mode():
    """Test using call_object_object_name mode."""

    def getter(object, name):
        object.count += 1
        return object.count, name

    m = Value()
    m.set_getattr_mode(GetAttr.CallObject_ObjectName, getter)

    class CustomGetAtom(Atom):
        val = m
        count = Int()

    a = CustomGetAtom()
    assert a.val == (1, "val")
    assert a.val == (2, "val")

    with pytest.raises(TypeError):
        m.set_getattr_mode(GetAttr.CallObject_ObjectName, 1)


def test_using_object_method_mode():
    """Test using object_method mode."""
    m = Int()
    m.set_getattr_mode(GetAttr.ObjectMethod, "getter")

    class CustomGetAtom(Atom):
        val = m
        count = Int()

        def getter(self):
            self.count += 1
            return self.count

    a = CustomGetAtom()
    assert a.val == 1
    assert a.val == 2

    with pytest.raises(TypeError):
        m.set_getattr_mode(GetAttr.ObjectMethod, 1)


def test_using_object_method_name_mode():
    """Test using object_method mode."""
    m = Value()
    m.set_getattr_mode(GetAttr.ObjectMethod_Name, "getter")

    class CustomGetAtom(Atom):
        val = m
        count = Int()

        def getter(self, name):
            self.count += 1
            return (self.count, name)

    a = CustomGetAtom()
    assert a.val == (1, "val")
    assert a.val == (2, "val")

    with pytest.raises(TypeError):
        m.set_getattr_mode(GetAttr.ObjectMethod_Name, 1)


def test_subclassing_member():
    """Test defining get in a Member subclass"""

    class ModuloInt(Int):
        def __init__(self):
            super(ModuloInt, self).__init__()
            mode = GetAttr.MemberMethod_Object
            self.set_getattr_mode(mode, "get")

        def get(self, obj):
            return self.get_slot(obj) % 2

    class GetTest(Atom):
        mi = ModuloInt()

    pvt = GetTest()
    mi = pvt.get_member("mi")
    assert mi.getattr_mode[0] == GetAttr.MemberMethod_Object
    pvt.mi = 2
    assert pvt.mi == 0
    pvt.mi = 3
    assert pvt.mi == 1

    pvt.mi = 2
    assert mi.do_getattr(pvt) == 0
    pvt.mi = 3
    assert mi.do_getattr(pvt) == 1

    with pytest.raises(TypeError):
        Int().set_getattr_mode(GetAttr.MemberMethod_Object, 1)


def test_handling_wrong_index():
    """Test handling a wrong index in the slot handler."""

    class SlotAtom(Atom):
        v = Value()

    sa = SlotAtom()

    assert SlotAtom.v.do_getattr(sa) is None

    SlotAtom.v.set_index(SlotAtom.v.index + 1)

    with pytest.raises(AttributeError):
        SlotAtom.v.do_getattr(sa)
