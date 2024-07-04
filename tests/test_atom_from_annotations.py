# ------------------------------------------------------------------------------
# Copyright (c) 2021-2023, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# ------------------------------------------------------------------------------
"""Test defining an atom class using typing annotations.

"""
import logging
import sys
from collections import defaultdict
from typing import (
    Any,
    Callable as TCallable,
    ClassVar,
    DefaultDict as TDefaultDict,
    Dict as TDict,
    Iterable,
    List as TList,
    Optional,
    Set as TSet,
    Tuple as TTuple,
    Type,
    TypeVar,
    Union,
)

import pytest

from atom.api import (
    Atom,
    Bool,
    Bytes,
    Callable,
    DefaultDict,
    Dict,
    Float,
    Instance,
    Int,
    List,
    Member,
    Set,
    Str,
    Subclass,
    Tuple,
    Typed,
    Value,
)
from atom.atom import set_default


def test_ignore_annotations():
    class A(Atom, use_annotations=False):
        a: int

    assert not hasattr(A, "a")


def test_reject_str_annotations():
    with pytest.raises(TypeError):

        class A(Atom, use_annotations=True):
            a: "int"


def test_ignore_class_var():
    class A(Atom, use_annotations=True):
        a: ClassVar[int] = 1

    assert not isinstance(A.a, Member)


@pytest.mark.skipif(
    sys.version_info < (3, 9), reason="Subscription of Members requires Python 3.9+"
)
def test_ignore_annotated_member():
    class A(Atom, use_annotations=True):
        a: List[int] = List(default=[1, 2, 3])

    assert A().a == [1, 2, 3]


def test_ignore_str_annotated_member():
    class A(Atom, use_annotations=True):
        b: "List[int]" = List(default=[1, 2, 3])

    assert A().b == [1, 2, 3]


@pytest.mark.skipif(
    sys.version_info < (3, 9), reason="Subscription of Members requires Python 3.9+"
)
def test_ignore_annotated_set_default():
    class A(Atom, use_annotations=True):
        a = Value()

    class B(A, use_annotations=True):
        a: Value[int] = set_default(1)

    assert B().a == 1


def test_ignore_str_annotated_set_default():
    class A(Atom, use_annotations=True):
        a = Value()

    class B(A, use_annotations=True):
        a: "Value[int]" = set_default(1)

    assert B().a == 1


def test_reject_bare_member_annotated_member():
    with pytest.raises(ValueError) as e:

        class A(Atom, use_annotations=True):
            a: List

    assert "field 'a' of 'A'" in e.exconly()
    assert e.value.__cause__


def test_reject_non_member_annotated_member():
    with pytest.raises(TypeError):

        class A(Atom, use_annotations=True):
            a: TList[int] = List(int, default=[1, 2, 3])


def test_reject_non_member_annotated_set_default():
    class A(Atom, use_annotations=True):
        a = Value()

    with pytest.raises(TypeError):

        class B(A, use_annotations=True):
            a: int = set_default(1)


@pytest.mark.parametrize(
    "annotation, member",
    [
        (bool, Bool),
        (int, Int),
        (float, Float),
        (str, Str),
        (bytes, Bytes),
        (Any, Value),
        (object, Value),
        (TCallable, Callable),
        (TCallable[[int], int], Callable),
        (logging.Logger, Typed),
        (Iterable, Instance),
        (Type[int], Subclass),
        (Type[TypeVar("T")], Subclass),
    ],
)
def test_annotation_use(annotation, member):
    class A(Atom, use_annotations=True):
        a: annotation

    assert isinstance(A.a, member)
    if member is Typed:
        assert A.a.validate_mode[1] == annotation
    elif member is Instance:
        assert A.a.validate_mode[1] == (annotation.__origin__,)
    elif member is Subclass:
        if isinstance(annotation.__args__[0], TypeVar):
            assert A.a.validate_mode[1] is object
        else:
            assert A.a.validate_mode[1] == annotation.__args__[0]
    else:
        assert A.a.default_value_mode == member().default_value_mode


@pytest.mark.parametrize(
    "annotation, member, validate_mode",
    [
        (Atom, Typed, Atom),
        (Union[int, str], Instance, (int, str)),
    ],
)
def test_union_in_annotation(annotation, member, validate_mode):
    class A(Atom, use_annotations=True):
        a: annotation

    assert isinstance(A.a, member)
    assert A.a.validate_mode[1] == validate_mode


@pytest.mark.parametrize(
    "annotation, member, depth",
    [
        (TList[int], List(), 0),
        (TList[int], List(Int()), 1),
        (TList[TList[int]], List(List()), 1),
        (TList[TList[int]], List(List(Int())), -1),
        (TSet[int], Set(), 0),
        (TSet[int], Set(Int()), 1),
        (TDict[int, int], Dict(), 0),
        (TDict[int, int], Dict(Int(), Int()), 1),
        (TDefaultDict[int, int], DefaultDict(Int(), Int()), 1),
        (TTuple[int], Tuple(), 0),
        (TTuple[int], Tuple(Int()), 1),
        (TTuple[int, ...], Tuple(Int()), 1),
        (TTuple[int, float], Tuple(), 1),
        (TTuple[tuple, int], Tuple(), 1),
    ]
    + (
        [
            (list[int], List(), 0),
            (list[int], List(Int()), 1),
            (list[list[int]], List(List()), 1),
            (list[list[int]], List(List(Int())), -1),
            (set[int], Set(), 0),
            (set[int], Set(Int()), 1),
            (dict[int, int], Dict(), 0),
            (dict[int, int], Dict(Int(), Int()), 1),
            (defaultdict[int, int], DefaultDict(Int(), Int()), 1),
            (tuple[int], Tuple(), 0),
            (tuple[int], Tuple(Int()), 1),
            (tuple[int, ...], Tuple(Int()), 1),
            (tuple[int, float], Tuple(), 1),
            (tuple[tuple, int], Tuple(), 1),
        ]
        if sys.version_info >= (3, 9)
        else []
    ),
)
def test_annotated_containers_no_default(annotation, member, depth):
    class A(Atom, use_annotations=True, type_containers=depth):
        a: annotation

    assert isinstance(A.a, type(member))
    if depth == 0:
        if isinstance(member, Dict):
            assert A.a.validate_mode[1] == (None, None)
        else:
            assert A.a.item is None
    else:
        if isinstance(member, Dict):
            for (k, v), (mk, mv) in zip(
                A.a.validate_mode[1:], member.validate_mode[1:]
            ):
                assert type(k) is type(mk)
                assert type(v) is type(mv)
        elif isinstance(member, DefaultDict):
            for (k, v, f), (mk, mv, mf) in zip(
                A.a.validate_mode[1:], member.validate_mode[1:]
            ):
                assert type(k) is type(mk)
                assert type(v) is type(mv)
                assert f(A()) == mf(A())
        else:
            assert type(A.a.item) is type(member.item)
            if isinstance(member.item, List):
                assert type(A.a.item.item) is type(member.item.item)


@pytest.mark.parametrize(
    "annotation, member, default",
    [
        (bool, Bool, True),
        (int, Int, 1),
        (float, Float, 1.0),
        (str, Str, "a"),
        (bytes, Bytes, b"a"),
        (Any, Value, 1),
        (Union[Any, None], Value, None),
        (object, Value, 2),
        (TCallable, Callable, lambda x: x),
        (TList, List, [1]),
        (TSet, Set, {1}),
        (TDict, Dict, {1: 2}),
        (TDefaultDict, DefaultDict, defaultdict(int, {1: 2})),
        (Optional[Iterable], Instance, None),
        (Type[int], Subclass, int),
    ]
    + (
        [
            (list, List, [1]),
            (set, Set, {1}),
            (dict, Dict, {1: 2}),
            (defaultdict, DefaultDict, defaultdict(int, {1: 2})),
        ]
        if sys.version_info >= (3, 9)
        else []
    ),
)
def test_annotations_with_default(annotation, member, default):
    class A(Atom, use_annotations=True):
        a: annotation = default

    assert isinstance(A.a, member)
    if member is Subclass:
        assert A.a.default_value_mode == member(int, default=default).default_value_mode
    elif member is not Instance:
        assert A.a.default_value_mode == member(default=default).default_value_mode


def test_annotations_no_default_for_instance():
    with pytest.raises(ValueError):

        class A(Atom, use_annotations=True):
            a: Iterable = []

    with pytest.raises(ValueError):

        class B(Atom, use_annotations=True):
            a: Optional[Iterable] = []
