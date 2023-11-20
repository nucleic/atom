# --------------------------------------------------------------------------------------
# Copyright (c) 2013-2023, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
"""Test the delegator member.

We need to test all behaviors:
    - get
    - post_get
    - set
    - post_set
    - post_validate
    - default_value
    - del

"""
import pytest

from atom.api import (
    Atom,
    DefaultValue,
    Delegator,
    GetAttr,
    Int,
    PostGetAttr,
    PostSetAttr,
    PostValidate,
    SetAttr,
    Validate,
)
from atom.catom import DelAttr


class TrackedInt(Int):
    """Member used to check that a Delegate does forward all calls."""

    def __init__(self, set_custom_modes=False):
        super(TrackedInt, self).__init__()
        self.called = []
        if set_custom_modes:
            mode = GetAttr.MemberMethod_Object
            self.set_getattr_mode(mode, "get")
            mode = PostGetAttr.MemberMethod_ObjectValue
            self.set_post_getattr_mode(mode, "post_get")
            mode = SetAttr.MemberMethod_ObjectValue
            self.set_setattr_mode(mode, "set")
            mode = PostSetAttr.MemberMethod_ObjectOldNew
            self.set_post_setattr_mode(mode, "post_set")
            mode = Validate.MemberMethod_ObjectOldNew
            self.set_validate_mode(mode, "validate")
            mode = PostValidate.MemberMethod_ObjectOldNew
            self.set_post_validate_mode(mode, "post_validate")
            mode = DefaultValue.MemberMethod_Object
            self.set_default_value_mode(mode, "default_value")

    def get(self, obj):
        self.called.append("get")
        return self.get_slot(obj)

    def post_get(self, obj, value):
        self.called.append("post_get")
        return value

    def set(self, obj, value):
        self.called.append("set")
        self.set_slot(obj, value)

    def post_set(self, obj, old, new):
        self.called.append("post_set")

    def validate(self, obj, old, new):
        self.called.append("validate")
        if not isinstance(new, int):
            raise TypeError()
        return new

    def post_validate(self, obj, old, new):
        self.called.append("post_validate")
        return new

    def default_value(self, obj):
        self.called.append("default_value")
        return 0

    def clone(self):
        # Re-implemented here to make sure that mode setting willl result
        # from the action of cloning the delegator
        return TrackedInt(False)


def test_delegator_behaviors():
    """Test that a Delegator does properly forward the behaviors."""

    class DelegateTest(Atom):
        d = Delegator(TrackedInt(True))

    # Test that behaviors are properly delegated
    dt = DelegateTest()
    assert dt.d == 0
    assert DelegateTest.d.delegate.called == [
        "default_value",
        "validate",
        "post_validate",
        "post_get",
    ]
    DelegateTest.d.delegate.called = []

    dt.d = 1
    assert DelegateTest.d.delegate.called == ["validate", "post_validate", "post_set"]
    assert dt.d == 1
    # Make sure they use the same slot
    assert DelegateTest.d.delegate.do_getattr(dt) == 1

    # Test also delegating get and set
    DelegateTest.d.delegate.called = []
    DelegateTest.d.set_getattr_mode(GetAttr.Delegate, DelegateTest.d.delegate)
    DelegateTest.d.set_setattr_mode(SetAttr.Delegate, DelegateTest.d.delegate)
    DelegateTest.d.set_delattr_mode(DelAttr.Delegate, DelegateTest.d.delegate)
    assert dt.d == 1
    assert DelegateTest.d.delegate.called == ["get", "validate", "post_validate"]
    dt.d = 2
    assert DelegateTest.d.delegate.called == [
        "get",
        "validate",
        "post_validate",
        "validate",
        "post_validate",
        "set",
    ]
    assert dt.d == 2

    # Test delegating del (This will cause an error because the validator will
    # get None)
    del dt.d
    with pytest.raises(TypeError):
        assert dt.d == 0


@pytest.mark.parametrize(
    "mode, func",
    [
        (GetAttr, "set_getattr_mode"),
        (SetAttr, "set_setattr_mode"),
        (DelAttr, "set_delattr_mode"),
        (PostGetAttr, "set_post_getattr_mode"),
        (PostSetAttr, "set_post_setattr_mode"),
        (Validate, "set_validate_mode"),
        (PostValidate, "set_post_validate_mode"),
    ],
)
def test_delegator_mode_args_validation(mode, func):
    """Test that a delegator properly validate the arguments when setting mode."""
    with pytest.raises(TypeError) as excinfo:
        getattr(Delegator(Int()), func)(getattr(mode, "Delegate"), None)
    assert "Member" in excinfo.exconly()


def test_delegator_methods():
    """Test manipulating a delegator.

    Mode setting methods are tested in cloning test

    """

    class DelegateTest(Atom):
        d = Delegator(TrackedInt(True))

    assert DelegateTest.d.name == "d"
    assert DelegateTest.d.delegate.name == "d"
    DelegateTest.d.set_name("e")
    assert DelegateTest.d.name == "e"
    assert DelegateTest.d.delegate.name == "e"

    assert DelegateTest.d.index == DelegateTest.d.delegate.index
    new_index = DelegateTest.d.index + 1
    DelegateTest.d.set_index(new_index)
    assert DelegateTest.d.index == new_index
    assert DelegateTest.d.delegate.index == new_index

    def observer(s, c):
        return None

    assert not DelegateTest.d.delegate.static_observers()
    DelegateTest.d.add_static_observer(observer)
    assert DelegateTest.d.delegate.static_observers()
    DelegateTest.d.remove_static_observer(observer)
    assert not DelegateTest.d.delegate.static_observers()


def test_delegator_cloning():
    """Test cloning a delegator member.

    Test that the delegate is also cloned and its mode properly configured.

    """

    class DelegateTest(Atom):
        tracked_int = TrackedInt(True)

        d = Delegator(tracked_int)

    # Checked that a cloned TrackedInt does not have any special mode set
    ti_clone = DelegateTest.tracked_int.clone()
    for m in (
        "validate_mode",
        "post_getattr_mode",
        "post_validate_mode",
        "post_setattr_mode",
    ):
        assert getattr(ti_clone, m)[1] is None

    d_clone = DelegateTest.d.clone()
    for m in (
        "validate_mode",
        "post_getattr_mode",
        "post_validate_mode",
        "post_setattr_mode",
    ):
        assert getattr(d_clone.delegate, m)[1] is not None
