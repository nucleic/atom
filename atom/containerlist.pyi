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

class ContainerList(Member[list[T], list[T]]):
    # No default
    @overload
    def __new__(
        cls, kind: None = None, default: Optional[list] = None
    ) -> ContainerList[Any]: ...
    @overload
    def __new__(cls, kind: Type[T], default: None = None) -> ContainerList[T]: ...
    @overload
    def __new__(
        cls, kind: Tuple[Type[T]], default: None = None
    ) -> ContainerList[T]: ...
    @overload
    def __new__(
        cls, kind: Tuple[Type[T], Type[T1]], default: None = None
    ) -> ContainerList[T | T1]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        default: None = None,
    ) -> ContainerList[T | T1 | T2]: ...
    @overload
    def __new__(
        cls, kind: Member[T, Any], default: None = None
    ) -> ContainerList[T]: ...
    # With default
    @overload
    def __new__(cls, kind: Type[T], default: list[T]) -> ContainerList[T]: ...
    @overload
    def __new__(cls, kind: Tuple[Type[T]], default: list[T]) -> ContainerList[T]: ...
    @overload
    def __new__(
        cls, kind: Tuple[Type[T], Type[T1]], default: list[T | T1]
    ) -> ContainerList[T | T1]: ...
    @overload
    def __new__(
        cls, kind: Tuple[Type[T], Type[T1]], default: list[T] | list[T1]
    ) -> ContainerList[T | T1]: ...
    @overload
    def __new__(
        cls, kind: Tuple[Type[T], Type[T1], Type[T2]], default: list[T | T1 | T2]
    ) -> ContainerList[T | T1 | T2]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        default: list[T | T1] | list[T | T2] | list[T1 | T2],
    ) -> ContainerList[T | T1 | T2]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        default: list[T] | list[T1] | list[T2],
    ) -> ContainerList[T | T1 | T2]: ...
    @overload
    def __new__(
        cls, kind: Member[T, Any], default: Optional[list[T]] = None
    ) -> ContainerList[T]: ...
