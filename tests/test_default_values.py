# --------------------------------------------------------------------------------------
# Copyright (c) 2013-2023, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
"""Tests for member validation handlers:

    no_op_handler: unused as far as I can tell
    static_handler
    list_handler
    dict_handler
    delegate_handler: not tested here
    call_object_handler: used for factory function or Typed/Instance with args
    call_object_object_handler: advanced used case not used internally
    call_object_object_name_handler: advanced used case not used internally
    object_method_handler
    object_method_name_handler: advanced used case not used internally
    member_method_object_handler

"""
import pytest

from atom.api import (
    Atom,
    Coerced,
    DefaultValue,
    Dict,
    FloatRange,
    ForwardInstance,
    ForwardSubclass,
    ForwardTyped,
    Instance,
    List,
    Member,
    Range,
    Set,
    Subclass,
    Typed,
    Value,
)


def test_no_op_handler():
    """Test the NoOp handler."""

    class A(Atom):
        v = Member()

    assert A.v.default_value_mode[0] == DefaultValue.NoOp
    assert A().v is None


@pytest.mark.parametrize(
    "member, expected",
    [
        (Value(1), 1),
        (Range(0), 0),
        (Range(high=0), 0),
        (Range(0, value=1), 1),
        (FloatRange(0.0), 0.0),
        (FloatRange(high=0.0), 0.0),
        (FloatRange(0.0, value=1.0), 1.0),
        (Subclass(float), float),
        (ForwardSubclass(lambda: float), float),
    ],
)
def test_static_handler(member, expected):
    """Test a static handler."""

    class StaticTest(Atom):
        v = member

    mode = (
        DefaultValue.MemberMethod_Object
        if isinstance(member, ForwardSubclass)
        else DefaultValue.Static
    )
    assert StaticTest.v.default_value_mode[0] == mode
    assert StaticTest().v == expected
    assert StaticTest.v.default_value_mode[0] == DefaultValue.Static


@pytest.mark.parametrize(
    "member, expect_error",
    [
        (Typed(int), False),
        (Typed(int, (), optional=False), False),
        (Typed(int, factory=lambda: 1, optional=False), False),
        (Instance(int), False),
        (Instance(int, (), optional=False), False),
        (Instance(int, factory=lambda: 1, optional=False), False),
        (Instance(int, optional=False), True),
        (ForwardTyped(lambda: int), False),
        (ForwardTyped(lambda: int, (), optional=False), False),
        (ForwardTyped(lambda: int, factory=lambda: 1, optional=False), False),
        (ForwardTyped(lambda: int, optional=False), True),
        (ForwardInstance(lambda: int), False),
        (ForwardInstance(lambda: int, (), optional=False), False),
        (ForwardInstance(lambda: int, factory=lambda: 1, optional=False), False),
        (ForwardInstance(lambda: int, optional=False), True),
    ],
)
def test_non_optional_handler(member, expect_error):
    """Test a static handler."""

    class NonOptionalTest(Atom):
        v = member

    if expect_error:
        assert NonOptionalTest.v.default_value_mode[0] == DefaultValue.NonOptional
        with pytest.raises(ValueError) as e:
            NonOptionalTest().v
        assert "is not optional but no default value" in str(e)
    else:
        NonOptionalTest().v

    if not expect_error:
        assert NonOptionalTest.v.default_value_mode[0] != DefaultValue.NonOptional


def test_list_handler():
    """Test that the list handler properly copies the default value."""

    class ListTest(Atom):
        no_default = List()
        default = List(default=["a"])

    assert ListTest.no_default.default_value_mode[0] == DefaultValue.List
    assert ListTest().no_default == []

    assert ListTest.default.default_value_mode[0] == DefaultValue.List
    default_value = ListTest.default.default_value_mode[1]
    assert ListTest().default == default_value
    assert ListTest().default is not default_value


def test_dict_handler():
    """Test that the dict handler properly copies the default value."""

    class DictTest(Atom):
        no_default = Dict()
        default = Dict(default={"a": 1})

    assert DictTest.no_default.default_value_mode[0] == DefaultValue.Dict
    assert DictTest().no_default == {}

    assert DictTest.default.default_value_mode[0] == DefaultValue.Dict
    default_value = DictTest.default.default_value_mode[1]
    assert DictTest().default == default_value
    assert DictTest().default is not default_value


def test_set_handler():
    """Test that the set handler properly copies the default value."""

    class SetTest(Atom):
        no_default = Set()
        default = Set(default={"a"})

    assert SetTest.no_default.default_value_mode[0] == DefaultValue.Set
    assert SetTest().no_default == set()

    assert SetTest.default.default_value_mode[0] == DefaultValue.Set
    default_value = SetTest.default.default_value_mode[1]
    assert SetTest().default == default_value
    assert SetTest().default is not default_value


@pytest.mark.parametrize(
    "member, expected, mode",
    [
        (Typed(int, ("101",), {"base": 2}), 5, DefaultValue.CallObject),
        (Typed(int, factory=lambda: int(5)), 5, DefaultValue.CallObject),
        (
            ForwardTyped(lambda: int, ("101",), {"base": 2}),
            5,
            DefaultValue.MemberMethod_Object,
        ),
        (ForwardTyped(lambda: int, factory=lambda: int(5)), 5, DefaultValue.CallObject),
        (Instance(int, ("101",), {"base": 2}), 5, DefaultValue.CallObject),
        (Instance(int, factory=lambda: int(5)), 5, DefaultValue.CallObject),
        (
            ForwardInstance(lambda: int, ("101",), {"base": 2}),
            5,
            DefaultValue.MemberMethod_Object,
        ),
        (
            ForwardInstance(lambda: int, factory=lambda: int(5)),
            5,
            DefaultValue.CallObject,
        ),
        (Value(factory=lambda: 5), 5, DefaultValue.CallObject),
        (Coerced((int, type(None)), coercer=int), None, DefaultValue.CallObject),
        (Coerced(int, ()), 0, DefaultValue.CallObject),
        (Coerced(int, factory=lambda: 5), 5, DefaultValue.CallObject),
    ],
)
def test_callobject_handler(member, expected, mode):
    """Test calling factory handler."""

    class CallTest(Atom):
        m = member

    assert CallTest.m.default_value_mode[0] == mode
    assert CallTest().m == expected
    # Called twice to call the resolved version of the default for forward members
    assert CallTest().m == expected
    assert CallTest.m.default_value_mode[0] == DefaultValue.CallObject


def test_callobject_object_handler():
    """Test the CallObject_Object mode."""
    mode = DefaultValue.CallObject_Object
    member = Value()
    member.set_default_value_mode(mode, lambda obj: id(obj))

    class A(Atom):
        m = member

    a1 = A()
    a2 = A()
    assert id(a1) == a1.m
    assert id(a2) == a2.m
    assert a1.m != a2.m


def test_callobject_object_name_handler():
    """Test the CallObject_ObjectName mode."""
    mode = DefaultValue.CallObject_ObjectName
    member = Value()
    member.set_default_value_mode(mode, lambda obj, name: (id(obj), name))

    class A(Atom):
        m = member

    a1 = A()
    a2 = A()
    assert id(a1), "m" == a1.m
    assert id(a2), "m" == a2.m
    assert a1.m != a2.m


def test_object_method_member_handler():
    """Test calling a method on the Atom object."""

    class DefaultMethodTest(Atom):
        v = Value()

        def _default_v(self):
            return 5

    assert DefaultMethodTest.v.default_value_mode[0] == DefaultValue.ObjectMethod
    assert DefaultMethodTest().v == 5


def test_member_method_object_handler():
    """Test subclassing a Member."""
    SENTINEL = object()

    class DefaultValueMember(Value):
        def __init__(self):
            super(DefaultValueMember, self).__init__()
            mode = DefaultValue.MemberMethod_Object
            self.set_default_value_mode(mode, "default_value")

        def default_value(self, atom):
            return SENTINEL

    class MemberTest(Atom):
        m = DefaultValueMember()

    assert MemberTest.m.default_value_mode[0] == DefaultValue.MemberMethod_Object
    assert MemberTest().m is SENTINEL
    assert MemberTest.m.do_default_value(MemberTest()) is SENTINEL


def test_object_method_name_handler():
    """Test object"""
    mode = DefaultValue.ObjectMethod_Name
    member = Value()
    member.set_default_value_mode(mode, "custom_default")

    class DefaultMethodTest(Atom):
        v = member

        def custom_default(self, name):
            return 5, name

    assert DefaultMethodTest().v == (5, "v")


@pytest.mark.parametrize(
    "mode, default",
    [
        (DefaultValue.List, 1),
        (DefaultValue.Dict, 1),
        (DefaultValue.Delegate, 1),
        (DefaultValue.CallObject, 1),
        (DefaultValue.CallObject_Object, 1),
        (DefaultValue.CallObject_ObjectName, 1),
        (DefaultValue.ObjectMethod, 1),
        (DefaultValue.ObjectMethod_Name, 1),
        (DefaultValue.MemberMethod_Object, 1),
    ],
)
def test_validating_default_value(mode, default):
    """Test that we validate the proposed default value when setting the mode."""
    with pytest.raises(TypeError):
        Member().set_default_value_mode(mode, default)
