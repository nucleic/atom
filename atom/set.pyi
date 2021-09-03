# --------------------------------------------------------------------------------------
# Copyright (c) 2021, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
from typing import Any, Tuple, Type, TypeVar, Optional, overload

from .catom import Member

T = TypeVar("T")
T1 = TypeVar("T1")
T2 = TypeVar("T2")

class Set(Member[set[T], set[T]]):
    @overload
    def __new__(cls, item: None = None, default: Optional[set] = None) -> Set[Any]: ...
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
    # With default
    # The splitting is necessary otherwise Mypy type inference fails
    @overload
    def __new__(cls, item: Type[T], default: set[T]) -> Set[T]: ...
    @overload
    def __new__(cls, item: Tuple[Type[T]], default: set[T]) -> Set[T]: ...
    @overload
    def __new__(
        cls, item: Tuple[Type[T], Type[T1]], default: set[T | T1]
    ) -> Set[T | T1]: ...
    @overload
    def __new__(
        cls, item: Tuple[Type[T], Type[T1]], default: set[T] | set[T1]
    ) -> Set[T | T1]: ...
    @overload
    def __new__(
        cls,
        item: Tuple[Type[T], Type[T1], Type[T2]],
        default: set[T | T1 | T2],
    ) -> Set[T | T1 | T2]: ...
    @overload
    def __new__(
        cls,
        item: Tuple[Type[T], Type[T1], Type[T2]],
        default: set[T | T1] | set[T | T2] | set[T1 | T2],
    ) -> Set[T | T1 | T2]: ...
    @overload
    def __new__(
        cls,
        item: Tuple[Type[T], Type[T1], Type[T2]],
        default: set[T] | set[T1] | set[T2],
    ) -> Set[T | T1 | T2]: ...
    @overload
    def __new__(cls, item: Member[T, Any], default: None = None) -> Set[T]: ...
    @overload
    def __new__(cls, item: Member[T, Any], default: set[T]) -> Set[T]: ...
