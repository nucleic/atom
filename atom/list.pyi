# --------------------------------------------------------------------------------------
# Copyright (c) 2021, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
from typing import Any, Optional, Tuple, Type, TypeVar, overload

from .catom import Member

T = TypeVar("T")
T1 = TypeVar("T1")
T2 = TypeVar("T2")

class List(Member[list[T], list[T]]):
    # No default
    @overload
    def __new__(
        cls, kind: None = None, default: Optional[list] = None
    ) -> List[Any]: ...
    @overload
    def __new__(cls, kind: Type[T], default: None = None) -> List[T]: ...
    @overload
    def __new__(cls, kind: Tuple[Type[T]], default: None = None) -> List[T]: ...
    @overload
    def __new__(
        cls, kind: Tuple[Type[T], Type[T1]], default: None = None
    ) -> List[T | T1]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        default: None = None,
    ) -> List[T | T1 | T2]: ...
    @overload
    def __new__(cls, kind: Member[T, Any], default: None = None) -> List[T]: ...
    # With default
    @overload
    def __new__(cls, kind: Type[T], default: list[T]) -> List[T]: ...
    @overload
    def __new__(cls, kind: Tuple[Type[T]], default: list[T]) -> List[T]: ...
    @overload
    def __new__(
        cls, kind: Tuple[Type[T], Type[T1]], default: list[T | T1]
    ) -> List[T | T1]: ...
    @overload
    def __new__(
        cls, kind: Tuple[Type[T], Type[T1]], default: list[T] | list[T1]
    ) -> List[T | T1]: ...
    @overload
    def __new__(
        cls, kind: Tuple[Type[T], Type[T1], Type[T2]], default: list[T | T1 | T2]
    ) -> List[T | T1 | T2]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        default: list[T | T1] | list[T | T2] | list[T1 | T2],
    ) -> List[T | T1 | T2]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        default: list[T] | list[T1] | list[T2],
    ) -> List[T | T1 | T2]: ...
    @overload
    def __new__(cls, kind: Member[T, Any], default: list[T] = None) -> List[T]: ...
