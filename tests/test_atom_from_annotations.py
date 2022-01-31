# ------------------------------------------------------------------------------
# Copyright (c) 2021, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# ------------------------------------------------------------------------------
"""Test defining an atom class using typing annotations.

"""
from typing import (
    Any,
    Callable as TCallable,
    Dict as TDict,
    Iterable,
    List as TList,
    Optional,
    Set as TSet,
)

import pytest

from atom.api import (
    Atom,
    Bool,
    Bytes,
    Callable,
    Dict,
    Float,
    Instance,
    Int,
    List,
    Set,
    Str,
    Value,
)


def test_ignore_annotations():
    class A(Atom, use_annotations=False):
        a: int

    assert not hasattr(A, "a")


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
        (Iterable, Instance),
    ],
)
def test_annotation_use(annotation, member):
    class A(Atom):
        a: annotation

    assert isinstance(A.a, member)
    if member is Instance:
        assert A.a.validate_mode[1] == (annotation.__origin__,)
    else:
        assert A.a.default_value_mode == member().default_value_mode


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
    ],
)
def test_annotated_containers_no_default(annotation, member, depth):
    class A(Atom, type_containers=depth):
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

        else:
            assert type(A.a.item) is type(member.item)  # noqa: E721
            if isinstance(member.item, List):
                assert type(A.a.item.item) is type(member.item.item)  # noqa: E721


@pytest.mark.parametrize(
    "annotation, member, default",
    [
        (bool, Bool, True),
        (int, Int, 1),
        (float, Float, 1.0),
        (str, Str, "a"),
        (bytes, Bytes, b"a"),
        (Any, Value, 1),
        (object, Value, 2),
        (TCallable, Callable, lambda x: x),
        (TList, List, [1]),
        (TSet, Set, {1}),
        (TDict, Dict, {1: 2}),
        (Optional[Iterable], Instance, None),
    ],
)
def test_annotations_with_default(annotation, member, default):
    class A(Atom):
        a: annotation = default

    assert isinstance(A.a, member)
    if member is not Instance:
        assert A.a.default_value_mode == member(default=default).default_value_mode


def test_annotations_no_default_for_instance():
    with pytest.raises(ValueError):

        class A(Atom):  # noqa
            a: Iterable = []

    with pytest.raises(ValueError):

        class B(Atom):  # noqa
            a: Optional[Iterable] = []
