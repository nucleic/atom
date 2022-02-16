# --------------------------------------------------------------------------------------
# Copyright (c) 2021, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
"""Test typing utilities."""
import sys
from collections.abc import Iterable
from typing import Dict, List, Optional, Set, Union

import pytest

from atom.typing_utils import extract_types, is_optional


@pytest.mark.parametrize(
    "ty, outputs",
    [
        (List[int], (list,)),
        (Dict[str, int], (dict,)),
        (Set[int], (set,)),
        (Optional[int], (int, type(None))),
        (Union[int, str], (int, str)),
        (Union[int, Optional[str]], (int, str, type(None))),
        (Union[int, Union[str, bytes]], (int, str, bytes)),
    ]
    + (
        [
            (list[int], (list,)),
            (dict[str, int], (dict,)),
            (set[int], (set,)),
            (Iterable[int], (Iterable,)),
        ]
        if sys.version_info >= (3, 9)
        else []
    )
    + ([(int | str, (int, str))] if sys.version_info >= (3, 10) else []),
)
def test_extract_types(ty, outputs):
    assert extract_types(ty) == outputs


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
