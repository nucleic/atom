# --------------------------------------------------------------------------------------
# Copyright (c) 2023, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
"""Test the getstate behaviors

    include_handler: tested here
    exclude_handler: tested here
    include_non_default_handler: tested here
    property_handler: tested in test_property
    member_method_object_handler: tested here
    object_method_name_handler: tested here

"""
import pickle

import pytest

from atom.api import (
    Atom,
    Bool,
    Constant,
    ForwardInstance,
    ForwardTyped,
    GetState,
    Instance,
    ReadOnly,
    Typed,
    Value,
)


@pytest.mark.parametrize(
    "member, mode",
    [
        (Value(), GetState.Include),
        (ReadOnly(), GetState.IncludeNonDefault),
        (Constant(1), GetState.Exclude),
        (Typed(int, optional=True), GetState.Include),
        (Typed(int, optional=False), GetState.IncludeNonDefault),
        (ForwardTyped(lambda: int, optional=True), GetState.Include),
        (ForwardTyped(lambda: int, optional=False), GetState.IncludeNonDefault),
        (Instance(int, optional=True), GetState.Include),
        (Instance(int, optional=False), GetState.IncludeNonDefault),
        (ForwardInstance(lambda: int, optional=True), GetState.Include),
        (ForwardInstance(lambda: int, optional=False), GetState.IncludeNonDefault),
    ],
)
def test_member_getstate_mode(member, mode):
    assert member.getstate_mode[0] == mode


class A(Atom):
    val = Value()


def test_using_include_handler():
    """Test using the include handler."""
    A.val.set_getstate_mode(GetState.Include, None)

    a = A()
    assert A.val.do_should_getstate(a) is True
    assert b"val" in pickle.dumps(a, 0)


def test_using_exclude_handler():
    """Test using the include handler."""
    A.val.set_getstate_mode(GetState.Exclude, None)

    a = A()
    assert A.val.do_should_getstate(a) is False
    assert b"val" not in pickle.dumps(a, 0)


def test_using_include_non_default_handler():
    """Test using the include handler."""

    class A(Atom):
        val = Value()
        val.set_getstate_mode(GetState.IncludeNonDefault, None)

    assert A.val.do_should_getstate(A()) is False
    assert A.val.do_should_getstate(A(val=1)) is True


def test_using_object_method_name():
    """Test using object_method mode."""

    class A(Atom):
        val = Value()
        val2 = Value()
        seen = Value()

        def _getstate_val(self, name: str) -> bool:
            self.seen = name
            return name == "val"

        def _getstate_val2(self, name: str) -> bool:
            self.seen = name
            return name == "val"

    a = A()
    assert A.val.do_should_getstate(a) is True
    assert a.seen == "val"
    assert A.val2.do_should_getstate(a) is False
    assert a.seen == "val2"

    with pytest.raises(TypeError):
        Value().set_getstate_mode(GetState.ObjectMethod_Name, 1)


def test_subclassing_member():
    """Test defining get in a Member subclass"""

    class CustomV(Value):
        def __init__(self):
            super(Value, self).__init__()
            self.set_getstate_mode(GetState.MemberMethod_Object, "getstate")

        def getstate(self, obj):
            return obj.pickle

    class A(Atom):
        v = CustomV()
        pickle = Bool()

    pvt = A()
    assert A.v.do_should_getstate(pvt) is False
    pvt.pickle = True
    assert A.v.do_should_getstate(pvt) is True

    with pytest.raises(TypeError):
        Value().set_getstate_mode(GetState.MemberMethod_Object, 1)


def test_handling_exception_in_getstate():
    """Test handling an exception while pickling."""

    class CustomV(Value):
        def __init__(self):
            super(Value, self).__init__()
            self.set_getstate_mode(GetState.MemberMethod_Object, "getstate")

        def getstate(self, obj):
            raise RuntimeError

    class A(Atom):
        v = CustomV()

    with pytest.raises(RuntimeError):
        pickle.dumps(A())
