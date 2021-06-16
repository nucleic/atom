# --------------------------------------------------------------------------------------
# Copyright (c) 2021, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
from typing import (
    Any,
    Callable,
    Dict,
    Optional,
    Tuple as TTuple,
    Type,
    TypeVar,
    Union,
    overload,
)

from .catom import Member

T = TypeVar("T")
T1 = TypeVar("T1")
T2 = TypeVar("T2")

class Tuple(Member[T, T]):
    @overload
    def __new__(self, kind: Type[T], default: TTuple[T, ...]) -> Tuple[T]: ...
    @overload
    def __new__(self, kind: TTuple[Type[T]], default: TTuple[T, ...]) -> Tuple[T]: ...
    @overload
    def __new__(
        self, kind: TTuple[Type[T], Type[T1]], default: TTuple[Union[T, T1], ...]
    ) -> Tuple[Union[T, T1]]: ...
    @overload
    def __new__(
        self,
        kind: TTuple[Type[T], Type[T1], Type[T2]],
        default: TTuple[Union[T, T1, T2]],
    ) -> Tuple[Union[T, T1, T2]]: ...
    @overload
    def __new__(self, kind: Member[T], default: TTuple[T]) -> Tuple[T]: ...
