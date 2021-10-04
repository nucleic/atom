# ------------------------------------------------------------------------------
# Copyright (c) 2021, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# ------------------------------------------------------------------------------
"""Test defining an atom class using typing annotations.

"""
from typing import Any, Callable
from collections.abc import Iterable

import pytest
from atom.api import (
    Atom,
    Bool,
    Bytes,
    DefaultValue,
    Float,
    Instance,
    Int,
    Str,
    Typed,
    Value,
    Callable as ACallable,
)


@pytest.mark.parametrize("ann", [object, Any])
def test_value_no_default(ann):
    class A(Atom):
        o: ann

    assert isinstance(A.o, Value)
    assert A.o.default_value_mode == (DefaultValue.Static, None)


# XXX test ignoring annotations


@pytest.mark.parametrize(
    "ann, member",
    [
        (int, Int),
        (float, Float),
        (str, Str),
        (bytes, Bytes),
        (bool, Bool),
        (Callable[[int], int], ACallable),
    ],
)
def test_scalars_no_default(ann, member):
    class A(Atom):
        m: ann

    assert isinstance(A.m, member)
    assert A.m.default_value_mode == member().default_value_mode


class Dummy:
    pass


@pytest.mark.parametrize(("ann, member"), [(Iterable, Instance)])
def test_instance(ann, member):
    class A(Atom):
        m: ann

    assert isinstance(A.m, member)
