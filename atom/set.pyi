# --------------------------------------------------------------------------------------
# Copyright (c) 2021-2023, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
from typing import Any, Optional, Set as TSet, Tuple, Type, TypeVar, overload

from .catom import Member

T = TypeVar("T")
T1 = TypeVar("T1")
T2 = TypeVar("T2")

class Set(Member[TSet[T], TSet[T]]):
    @overload
    def __new__(
        cls, item: None = None, default: Optional[TSet[Any]] = None
    ) -> Set[Any]: ...
    @overload
    def __new__(cls, item: Type[T], default: None = None) -> Set[T]: ...
    @overload
    def __new__(cls, item: Tuple[Type[T]], default: None = None) -> Set[T]: ...
    @overload
    def __new__(
        cls, item: Tuple[Type[T], Type[T1]], default: None = None
    ) -> Set[T | T1]: ...
    @overload
    def __new__(
        cls,
        item: Tuple[Type[T], Type[T1], Type[T2]],
        default: None = None,
    ) -> Set[T | T1 | T2]: ...
    @overload
    def __new__(cls, item: Member[T, Any], default: None = None) -> Set[T]: ...
    # With default
    # The splitting is necessary otherwise Mypy type inference fails
    @overload
    def __new__(cls, item: Type[T], default: TSet[T]) -> Set[T]: ...
    @overload
    def __new__(cls, item: Tuple[Type[T]], default: TSet[T]) -> Set[T]: ...
    @overload
    def __new__(
        cls, item: Tuple[Type[T], Type[T1]], default: TSet[T | T1]
    ) -> Set[T | T1]: ...
    @overload
    def __new__(
        cls, item: Tuple[Type[T], Type[T1]], default: TSet[T] | TSet[T1]
    ) -> Set[T | T1]: ...
    @overload
    def __new__(
        cls,
        item: Tuple[Type[T], Type[T1], Type[T2]],
        default: TSet[T | T1 | T2],
    ) -> Set[T | T1 | T2]: ...
    @overload
    def __new__(
        cls,
        item: Tuple[Type[T], Type[T1], Type[T2]],
        default: TSet[T | T1] | TSet[T | T2] | TSet[T1 | T2],
    ) -> Set[T | T1 | T2]: ...
    @overload
    def __new__(
        cls,
        item: Tuple[Type[T], Type[T1], Type[T2]],
        default: TSet[T] | TSet[T1] | TSet[T2],
    ) -> Set[T | T1 | T2]: ...
    @overload
    def __new__(cls, item: Member[T, Any], default: TSet[T]) -> Set[T]: ...
