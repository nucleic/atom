#------------------------------------------------------------------------------
# Copyright (c) 2013-2017, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
"""Tests for member validation handlers:

    no_op_handler: unused as far as I can tell
    static_handler
    list_handler
    dict_handler
    delegate_handler: not tested here
    call_object_handler: used for factory function or Typed/Instance with args
    call_object_object_handler: unused as far as I can tell
    call_object_object_name_handler: unused as far as I can tell
    object_method_handler
    object_method_name_handler: unused as far as I can tell
    member_method_object_handler

"""
import pytest
from atom.compat import int

from atom.api import (Atom, Value, Range, FloatRange, List, Dict, Typed,
                      ForwardTyped, Instance, ForwardInstance, Coerced,
                      ForwardSubclass, DefaultValue)


@pytest.mark.parametrize("member, expected",
                         [(Value(1), 1),
                          (Range(0), 0),
                          (Range(high=0), 0),
                          (Range(0, value=1), 1),
                          (FloatRange(0.0), 0.0),
                          (FloatRange(high=0.0), 0.0),
                          (FloatRange(0.0, value=1.0), 1.0),
                          (ForwardSubclass(lambda: float), float)])
def test_static_handler(member, expected):
    """Test a static handler.

    """
    class StaticTest(Atom):
        v = member

    mode = (DefaultValue.MemberMethod_Object
            if isinstance(member, ForwardSubclass) else
            DefaultValue.Static)
    assert StaticTest.v.default_value_mode[0] == mode
    assert StaticTest().v == expected
    assert StaticTest.v.default_value_mode[0] == DefaultValue.Static


def test_list_handler():
    """Test that the list handler properly copies the default value.

    """
    class ListTest(Atom):
        no_default = List()
        default = List(default=['a'])

    assert ListTest.no_default.default_value_mode[0] == DefaultValue.List
    assert ListTest().no_default == []

    assert ListTest.default.default_value_mode[0] == DefaultValue.List
    default_value = ListTest.default.default_value_mode[1]
    assert ListTest().default == default_value
    assert ListTest().default is not default_value


def test_dict_handler():
    """Test that the dict handler properly copies the default value.

    """
    class DictTest(Atom):
        no_default = Dict()
        default = Dict(default={'a': 1})

    assert DictTest.no_default.default_value_mode[0] == DefaultValue.Dict
    assert DictTest().no_default == dict()

    assert DictTest.default.default_value_mode[0] == DefaultValue.Dict
    default_value = DictTest.default.default_value_mode[1]
    assert DictTest().default == default_value
    assert DictTest().default is not default_value


@pytest.mark.parametrize("member, expected, mode",
                         [(Typed(int, ('101',), dict(base=2)), 5,
                           DefaultValue.CallObject),
                          (Typed(int, factory=lambda: int(5)), 5,
                           DefaultValue.CallObject),
                          (ForwardTyped(lambda: int, ('101',), dict(base=2)),
                           5, DefaultValue.MemberMethod_Object),
                          (ForwardTyped(lambda: int, factory=lambda: int(5)),
                           5, DefaultValue.CallObject),
                          (Instance(int, ('101',), dict(base=2)), 5,
                           DefaultValue.CallObject),
                          (Instance(int, factory=lambda: int(5)), 5,
                           DefaultValue.CallObject),
                          (ForwardInstance(lambda: int,
                                           ('101',), dict(base=2)),
                           5, DefaultValue.MemberMethod_Object),
                          (ForwardInstance(lambda: int,
                                           factory=lambda: int(5)),
                           5, DefaultValue.CallObject),
                          (Value(factory=lambda: 5), 5,
                           DefaultValue.CallObject),
                          (Coerced(int, factory=lambda: 5), 5,
                           DefaultValue.CallObject)])
def test_call_object_handler(member, expected, mode):
    """Test calling factory handler.

    """
    class CallTest(Atom):

        m = member

    assert CallTest.m.default_value_mode[0] == mode
    assert CallTest().m == expected
    assert CallTest.m.default_value_mode[0] == DefaultValue.CallObject


def test_object_method_member_handler():
    """Test calling a method on the Atom object.

    """
    class DefaultMethodTest(Atom):
        v = Value()

        def _default_v(self):
            return 5

    assert (DefaultMethodTest.v.default_value_mode[0] ==
            DefaultValue.ObjectMethod)
    assert DefaultMethodTest().v == 5


def test_member_method_object_handler():
    """Test subclassing a Member.

    """
    SENTINEL = object()

    class DefaultValueMember(Value):

        def __init__(self):
            super(DefaultValueMember, self).__init__()
            mode = DefaultValue.MemberMethod_Object
            self.set_default_value_mode(mode, 'default_value')

        def default_value(self, atom):
            return SENTINEL

    class MemberTest(Atom):

        m = DefaultValueMember()

    assert (MemberTest.m.default_value_mode[0] ==
            DefaultValue.MemberMethod_Object)
    assert MemberTest().m is SENTINEL
    assert MemberTest.m.do_default_value(MemberTest()) is SENTINEL
