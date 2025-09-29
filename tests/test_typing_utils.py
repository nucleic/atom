# --------------------------------------------------------------------------------------
# Copyright (c) 2021-2025, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
"""Test typing utilities."""

from collections.abc import Iterable
from typing import Dict, List, Literal, NewType, Optional, Set, Tuple, TypeVar, Union

import pytest

from atom.typing_utils import extract_types, is_optional

T = TypeVar("T")
U = TypeVar("U", bound=int)
UU = TypeVar("UU", bound=Union[int, str])
V = TypeVar("V", int, float)
W = TypeVar("W", contravariant=True)
NT = NewType("NT", int)
NNT = NewType("NNT", NT)


@pytest.mark.parametrize(
    "ty, outputs",
    [
        (Tuple[int], (tuple,)),
        (List[int], (list,)),
        (Dict[str, int], (dict,)),
        (Set[int], (set,)),
        (Optional[int], (int, type(None))),
        (Union[int, str], (int, str)),
        (Union[int, Optional[str]], (int, str, type(None))),
        (Union[int, Union[str, bytes]], (int, str, bytes)),
        (list[int], (list,)),
        (dict[str, int], (dict,)),
        (set[int], (set,)),
        (Iterable[int], (Iterable,)),
        (int | str, (int, str)),
        (NT, (int,)),
        (NNT, (int,)),
    ],
)
def test_extract_types(ty, outputs):
    assert extract_types(ty) == outputs


def test_extract_types_for_type_vars():
    assert extract_types(T) == (object,)
    assert extract_types(U) == (int,)
    assert extract_types(UU) == (int, str)
    with pytest.raises(ValueError) as e:
        extract_types(V)
    assert "Constraints" in e.exconly()
    with pytest.raises(ValueError) as e:
        extract_types(W)
    assert "contravariant" in e.exconly()


@pytest.mark.parametrize(
    "ty, outputs",
    [
        (Optional[int], (True, (int,))),
        (Union[int, str], (False, (int, str))),
        (Union[int, Optional[str]], (True, (int, str))),
    ],
)
def test_is_optional(ty, outputs):
    assert is_optional(extract_types(ty)) == outputs


def test_reject_str_annotations():
    with pytest.raises(TypeError):
        extract_types("int")


def test_reject_literal():
    with pytest.raises(TypeError):
        extract_types(Literal[1])
