# --------------------------------------------------------------------------------------
# Copyright (c) 2021, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
from typing import Any, Set as TSet, Tuple, Type, TypeVar, Optional, Union, overload

from .catom import Member

T = TypeVar("T")
T1 = TypeVar("T1")
T2 = TypeVar("T2")

class Set(Member[TSet[T], TSet[T]]):
    @overload
    def __new__(cls, kind: None = None, default: Optional[TSet] = None) -> Set[Any]: ...
    @overload
    def __new__(cls, kind: Type[T], default: Optional[TSet[T]] = None) -> Set[T]: ...
    @overload
    def __new__(
        cls, kind: Tuple[Type[T]], default: Optional[TSet[T]] = None
    ) -> Set[T]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1]],
        default: Optional[TSet[Union[T, T1]]] = None,
    ) -> Set[Union[T, T1]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        default: Optional[TSet[Union[T, T1, T2]]] = None,
    ) -> Set[Union[T, T1, T2]]: ...
    @overload
    def __new__(
        cls, kind: Member[T, Any], default: Optional[TSet[T]] = None
    ) -> Set[T]: ...
