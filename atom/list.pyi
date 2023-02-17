# --------------------------------------------------------------------------------------
# Copyright (c) 2021-2023, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
from typing import Any, List as TList, Optional, Tuple, Type, TypeVar, overload

from .catom import Member

T = TypeVar("T")
T1 = TypeVar("T1")
T2 = TypeVar("T2")

class List(Member[TList[T], TList[T]]):
    # No default
    @overload
    def __new__(
        cls, kind: None = None, default: Optional[TList[Any]] = None
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
    def __new__(cls, kind: Type[T], default: TList[T]) -> List[T]: ...
    @overload
    def __new__(cls, kind: Tuple[Type[T]], default: TList[T]) -> List[T]: ...
    @overload
    def __new__(
        cls, kind: Tuple[Type[T], Type[T1]], default: TList[T | T1]
    ) -> List[T | T1]: ...
    @overload
    def __new__(
        cls, kind: Tuple[Type[T], Type[T1]], default: TList[T] | TList[T1]
    ) -> List[T | T1]: ...
    @overload
    def __new__(
        cls, kind: Tuple[Type[T], Type[T1], Type[T2]], default: TList[T | T1 | T2]
    ) -> List[T | T1 | T2]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        default: TList[T | T1] | TList[T | T2] | TList[T1 | T2],
    ) -> List[T | T1 | T2]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        default: TList[T] | TList[T1] | TList[T2],
    ) -> List[T | T1 | T2]: ...
    @overload
    def __new__(cls, kind: Member[T, Any], default: TList[T]) -> List[T]: ...
