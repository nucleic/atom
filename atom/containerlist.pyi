# --------------------------------------------------------------------------------------
# Copyright (c) 2021, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
from typing import Any, List, Optional, Tuple, Type, TypeVar, overload

from .catom import Member

T = TypeVar("T")
T1 = TypeVar("T1")
T2 = TypeVar("T2")

class ContainerList(Member[List[T], List[T]]):
    # No default
    @overload
    def __new__(
        cls, kind: None = None, default: Optional[List[Any]] = None
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
    def __new__(cls, kind: Type[T], default: List[T]) -> ContainerList[T]: ...
    @overload
    def __new__(cls, kind: Tuple[Type[T]], default: List[T]) -> ContainerList[T]: ...
    @overload
    def __new__(
        cls, kind: Tuple[Type[T], Type[T1]], default: List[T | T1]
    ) -> ContainerList[T | T1]: ...
    @overload
    def __new__(
        cls, kind: Tuple[Type[T], Type[T1]], default: List[T] | List[T1]
    ) -> ContainerList[T | T1]: ...
    @overload
    def __new__(
        cls, kind: Tuple[Type[T], Type[T1], Type[T2]], default: List[T | T1 | T2]
    ) -> ContainerList[T | T1 | T2]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        default: List[T | T1] | List[T | T2] | List[T1 | T2],
    ) -> ContainerList[T | T1 | T2]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        default: List[T] | List[T1] | List[T2],
    ) -> ContainerList[T | T1 | T2]: ...
    @overload
    def __new__(
        cls, kind: Member[T, Any], default: Optional[List[T]] = None
    ) -> ContainerList[T]: ...
