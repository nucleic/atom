# ------------------------------------------------------------------------------------------------------
# Copyright (c) 2013-2023, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# ------------------------------------------------------------------------------------------------------
"""Tests for member validation handlers:

    no_op_handler
    bool_handler
    int_handler
    int_promote_handler
    long_handler
    long_promote_handler
    float_handler
    float_promote_handler
    str_handler
    str_promote_handler
    unicode_handler
    unicode_promote_handler
    tuple_handler
    list_handler
    container_list_handler
    set_handler
    dict_handler
    instance_handler
    typed_handler
    subclass_handler
    enum_handler
    callable_handler
    float_range_handler
    range_handler
    coerced_handler
    delegate_handler: not tested here
    object_method_old_new_handler: used when defining validate on Atom subclass
    object_method_name_old_new_handler: unused as far as I can tell
    member_method_object_old_new_handler: used in ForwardType/Instance/Subclass

"""
import sys
from typing import List as TList, Optional, Sequence, Set as TSet, Union

import pytest

from atom.api import (
    Atom,
    Bool,
    Bytes,
    Callable,
    CAtom,
    Coerced,
    Constant,
    ContainerList,
    Delegator,
    Dict,
    Enum,
    Event,
    Float,
    FloatRange,
    ForwardInstance,
    ForwardSubclass,
    ForwardTyped,
    Instance,
    Int,
    List,
    Range,
    ReadOnly,
    Set,
    Str,
    Subclass,
    Tuple,
    Typed,
    Validate,
    Value,
)


def test_no_op_validation():
    """Test the no-op handler."""
    a = Atom()
    m = Value()
    m.set_validate_mode(Validate.NoOp, None)

    for value in (1, 1.0, "", [], {}):
        assert m.do_validate(a, None, value) == value


def c(x: object) -> int:
    return int(str(x), 2)


@pytest.mark.parametrize(
    "member, set_values, values, raising_values",
    [
        (Value(), ["a", 1, None], ["a", 1, None], []),
        (ReadOnly(int), [1], [1], [1.0]),
        (Bool(), [True, False], [True, False], "r"),
        (Int(strict=True), [1], [1], [1.0]),
        (Int(strict=False), [1, 1.0, int(1)], 3 * [1], ["a"]),
        (Range(0, 2), [0, 2], [0, 2], [-1, 3, ""]),
        (Range(2, 0), [0, 2], [0, 2], [-1, 3]),
        (Range(0), [0, 3], [0, 3], [-1]),
        (Range(high=2), [-1, 2], [-1, 2], [3]),
        (
            Range(sys.maxsize, sys.maxsize + 2),
            [sys.maxsize, sys.maxsize + 2],
            [sys.maxsize, sys.maxsize + 2],
            [sys.maxsize - 1, sys.maxsize + 3],
        ),
        (Float(), [1, int(1), 1.1], [1.0, 1.0, 1.1], [""]),
        (Float(strict=True), [1.1], [1.1], [1]),
        (FloatRange(0.0, 0.5), [0.0, 0.5], [0.0, 0.5], [-0.1, 0.6]),
        (FloatRange(0.5, 0.0), [0.0, 0.5], [0.0, 0.5], [-0.1, 0.6]),
        (FloatRange(0.0), [0.0, 0.6], [0.0, 0.6], [-0.1, ""]),
        (FloatRange(high=0.5), [-0.3, 0.5], [-0.3, 0.5], [0.6]),
        (FloatRange(1.0, 10.0, strict=True), [1.0, 3.7], [1.0, 3.7], [2, 4, 0, -11]),
        (Bytes(strict=False), [b"a", "a"], [b"a"] * 2, [1]),
        (Bytes(), [b"a"], [b"a"], ["a"]),
        (Str(strict=False), [b"a", "a"], ["a"] * 2, [1]),
        (Str(), ["a"], ["a"], [b"a"]),
        (Enum(1, 2, "a"), [1, 2, "a"], [1, 2, "a"], [3]),
        (Callable(), [int, None], [int, None], [1]),
        # 3.9 subs and 3.10 union tests in test_typing_utils are sufficient
        (Coerced(set), [{1}, [1], (1,)], [{1}] * 3, [1]),
        (Coerced(int, coercer=c), ["101"], [5], []),
        (Coerced((int, float), coercer=c), ["101"], [5], []),
        (Coerced(int, coercer=lambda x: []), [], [], [""]),  # type: ignore
        (Coerced(TSet[int]), [{1}, [1], (1,)], [{1}] * 3, [1]),
        (Tuple(), [(1,)], [(1,)], [[1]]),
        (Tuple(Int()), [(1,)], [(1,)], [(1.0,)]),
        (Tuple(int), [(1,)], [(1,)], [(1.0,), (None,)]),
        (Tuple(TSet[int]), [({1},)], [({1},)], [(1.0,), (None,)]),
        (Tuple(Optional[int]), [(1, None)], [(1, None)], [("",)]),
        (List(), [[1]], [[1]], [(1,)]),
        (List(Int()), [[1]], [[1]], [[1.0]]),
        (List(float), [[1.0]], [[1.0]], [[1], [None]]),
        (List((int, float)), [[1, 1.0]], [[1, 1.0]], [[""]]),
        (List(TSet[int]), [[{1}]], [[{1}]], [[1], [None]]),
        (List(Optional[int]), [[1, None]], [[1, None]], [[""]]),
        (ContainerList(), [[1]], [[1]], [(1,)]),
        (ContainerList(Int()), [[1]], [[1]], [[1.0]]),
        (ContainerList(float), [[1.0]], [[1.0]], [[1], [None]]),
        (ContainerList((int, float)), [[1, 1.0]], [[1, 1.0]], [[""]]),
        (ContainerList(TSet[int]), [[{1}]], [[{1}]], [[1], [None]]),
        (ContainerList(Optional[int]), [[1, None]], [[1, None]], [[""]]),
        (Set(), [{1}], [{1}], [()]),
        (Set(Int()), [{1}], [{1}], [{""}]),
        (Set(item=Int()), [{1}], [{1}], [{""}]),
        (Set(int), [{1}], [{1}], [{""}, {None}]),
        (Set(Optional[int]), [{1, None}], [{1, None}], [[1]]),
        (Dict(), [{1: 2}], [{1: 2}], [()]),
        (Dict(Int()), [{1: 2}], [{1: 2}], [{"": 2}]),
        (Dict(value=Int()), [{1: 2}], [{1: 2}], [{2: ""}]),
        (Dict(int, int), [{1: 2}], [{1: 2}], [{"": 2}, {2: ""}, {None: 2}, {2: None}]),
        (Dict(Optional[int]), [{None: 1, 1: 2}], [{None: 1, 1: 2}], [[1]]),
        (Instance((int, float)), [1, 2.0, None], [1, 2.0, None], [""]),
        (Instance((int, float), ()), [1, 2.0], [1, 2.0], ["", None]),
        (
            Instance((int, float), (), optional=True),
            [1, 2.0, None],
            [1, 2.0, None],
            [""],
        ),
        (
            Instance(Optional[Union[int, float]], ()),
            [1, 2.0, None],
            [1, 2.0, None],
            [""],
        ),
        (Instance((int, float), optional=False), [1, 2.0], [1, 2.0], [None, ""]),
        (Instance(TList[int], optional=False), [[1]], [[1]], [None, ""]),
        (Instance(Sequence[int], optional=False), [[1]], [[1]], [None, 1]),
        (ForwardInstance(lambda: (int, float)), [1, 2.0, None], [1, 2.0, None], [""]),
        (ForwardInstance(lambda: (int, float), ()), [1, 2.0], [1, 2.0], ["", None]),
        (
            ForwardInstance(lambda: (int, float), (), optional=True),
            [1, 2.0, None],
            [1, 2.0, None],
            [""],
        ),
        (
            ForwardInstance(lambda: Optional[Union[int, float]], ()),
            [1, 2.0, None],
            [1, 2.0, None],
            [""],
        ),
        (
            ForwardInstance(lambda: (int, float), optional=False),
            [1, 2.0],
            [1, 2.0],
            [None, ""],
        ),
        (Typed(float), [1.0, None], [1.0, None], [1]),
        (Typed(float, ()), [1.0], [1.0], [1, None]),
        (Typed(float, (), optional=True), [1.0, None], [1.0, None], [1]),
        (Typed(Optional[float], ()), [1.0, None], [1.0, None], [1]),
        (Typed(float, optional=False), [1.0], [1.0], [1, None]),
        (ForwardTyped(lambda: float), [1.0, None], [1.0, None], [1]),
        (ForwardTyped(lambda: float, ()), [1.0], [1.0], [1, None]),
        (ForwardTyped(lambda: float, (), optional=True), [1.0, None], [1.0, None], [1]),
        (ForwardTyped(lambda: float, optional=False), [1.0], [1.0], [1, None]),
        (Subclass(CAtom), [Atom], [Atom], [int, 1]),
        (Subclass((CAtom, float)), [Atom], [Atom], [int, 1]),
        (ForwardSubclass(lambda: CAtom), [Atom], [Atom], [int]),
    ],
)
def test_validation_modes(member, set_values, values, raising_values):
    """Test the validation modes."""

    class MemberTest(Atom):
        m = member

    tester = MemberTest()
    for sv, v in zip(set_values, values):
        tester.m = sv
        assert tester.m == v

    for rv in raising_values:
        with pytest.raises(
            OverflowError
            if (isinstance(member, Int) and isinstance(rv, float) and rv > 2**32)
            else ValueError
            if isinstance(member, Enum)
            else TypeError
        ):
            tester.m = rv


@pytest.mark.parametrize(
    "members, value",
    [
        ((List(Int()), List()), [1]),
        ((ContainerList(Int()), ContainerList()), [1]),
        ((Dict(Int(), Int()), Dict()), {1: 1}),
    ],
)
def test_validating_container_subclasses(members, value):
    """Ensure that we can pass atom containers to members."""

    class MemberTest(Atom):
        m1 = members[0]
        m2 = members[1]

    tester = MemberTest()
    tester.m1 = value
    tester.m2 = tester.m1


@pytest.mark.parametrize(
    "member, mode, arg, msg",
    [
        (List(), "List", 1, "Member or None"),
        (Tuple(), "Tuple", 1, "Member or None"),
        (ContainerList(), "ContainerList", 1, "Member or None"),
        (Set(), "Set", 1, "Member or None"),
        (Dict(), "Dict", 1, "2-tuple of Member or None"),
        (Dict(), "Dict", (), "2-tuple of Member or None"),
        (Dict(), "Dict", (1, None), "2-tuple of Member or None"),
        (Dict(), "Dict", (None, 1), "2-tuple of Member or None"),
        (Typed(int), "Typed", 1, "type"),
        (Enum(1, 2), "Enum", 1, "sequence"),
        (FloatRange(), "FloatRange", 1, "2-tuple of float or None"),
        (FloatRange(), "FloatRange", (), "2-tuple of float or None"),
        (FloatRange(), "FloatRange", ("", None), "2-tuple of float or None"),
        (FloatRange(), "FloatRange", (None, ""), "2-tuple of float or None"),
        (Range(), "Range", 1, "2-tuple of int or None"),
        (Range(), "Range", (), "2-tuple of int or None"),
        (Range(), "Range", ("", None), "2-tuple of int or None"),
        (Range(), "Range", (None, ""), "2-tuple of int or None"),
        (Coerced(int), "Coerced", 1, "2-tuple of (type, callable)"),
        (Coerced(int), "Coerced", (), "2-tuple of (type, callable)"),
        (Coerced(int), "Coerced", (int, 1), "2-tuple of (type, callable)"),
        (Instance(int), "Instance", 1, "type or tuple of types"),
        (Instance(int), "Instance", (int, 1), "type or tuple of types"),
        (Subclass(int), "Instance", 1, "type or tuple of types"),
        (Subclass(int), "Instance", (int, 1), "type or tuple of types"),
        (Delegator(Int()), "Delegate", 1, "Member"),
    ],
)
def test_handling_wrong_context(member, mode, arg, msg):
    """Test handling wrong args to members leading to wrong validate behaviors."""
    with pytest.raises(TypeError) as excinfo:
        member.set_validate_mode(getattr(Validate, mode), arg)
    assert msg in excinfo.exconly()


def test_event_validation():
    """Test validating the payload of an Event."""

    class EventValidationTest(Atom):
        ev_member = Event(Int())

        ev_type = Event(int)

    evt = EventValidationTest()

    evt.ev_member = 1
    evt.ev_type = 1
    with pytest.raises(TypeError):
        evt.ev_member = 1.0
    with pytest.raises(TypeError):
        evt.ev_type = 1.0


def test_constant_validation():
    """Test validating a constant."""

    class A(Atom):
        c = Constant(kind=int)

        def _default_c(self):
            return id(self)

    A().c

    class B(A):
        def _default_c(self):
            return str(super()._default_c())

    with pytest.raises(TypeError):
        B().c


def no_atom():
    """Set a member with a no-op validator."""

    class NoOpValAtom(Atom):
        v = Value()
        v.set_validate_mode(Validate.NoOp, None)

    return NoOpValAtom()


def om_on_atom():
    """Use an object method to customize validate."""

    class ValidatorTest(Atom):
        v = Value(0)

        def _validate_v(self, old, new):
            if not isinstance(new, int):
                raise TypeError()
            if old is not None and new != old + 1:
                raise ValueError()
            return new

    return ValidatorTest()


def om_non_atom():
    """Use an object method taking the member name to customize validate."""

    class ValidatorTest(Atom):
        v = Value(0)
        v.set_validate_mode(Validate.ObjectMethod_NameOldNew, "validate_v")

        def validate_v(self, name, old, new):
            if not isinstance(new, int):
                raise TypeError()
            if old is not None and new != old + 1:
                raise ValueError()
            return new

    return ValidatorTest()


def mm_oon_atom():
    """Use a member method to customize validate."""

    class CustomValue(Value):
        def __init__(self):
            super(CustomValue, self).__init__(0)
            self.set_validate_mode(Validate.MemberMethod_ObjectOldNew, "validate")

        def validate(self, object, old, new):
            if not isinstance(new, int):
                raise TypeError()
            if old is not None and new != old + 1:
                raise ValueError()
            return new

    class ValidatorTest(Atom):
        v = CustomValue()

    return ValidatorTest()


@pytest.mark.parametrize(
    "mode, factory",
    [
        ("NoOp", no_atom),
        ("ObjectMethod_OldNew", om_on_atom),
        ("ObjectMethod_NameOldNew", om_non_atom),
        ("MemberMethod_ObjectOldNew", mm_oon_atom),
    ],
)
def test_custom_validate(mode, factory):
    """Test specifying a specific validator in the Atom and using do_validate."""
    v = factory()
    assert type(v).v.validate_mode[0] == getattr(Validate, mode)
    assert v.v == (0 if mode != "NoOp" else None)
    if mode == "NoOp":
        return

    v.v = 1
    assert v.v == 1

    with pytest.raises(TypeError):
        v.v = None
    assert v.v == 1
    with pytest.raises(ValueError):
        v.v = 4

    v_member = type(factory()).v
    with pytest.raises(TypeError):
        v_member.do_validate(v, 1, None)
    assert v.v == 1
    with pytest.raises(ValueError):
        v_member.do_full_validate(v, 1, 4)

    with pytest.raises(TypeError) as excinfo:
        type(v).v.set_validate_mode(getattr(Validate, mode), 1)
    assert "str" in excinfo.exconly()
