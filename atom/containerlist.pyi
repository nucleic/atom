# --------------------------------------------------------------------------------------
# Copyright (c) 2021, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
from typing import Any, List as TList, Optional, Tuple, Type, TypeVar, Union, overload

from .catom import Member, atomclist

T = TypeVar("T")
T1 = TypeVar("T1")
T2 = TypeVar("T2")

class ContainerList(Member[atomclist[T], TList[T]]):
    @overload
    def __new__(
        cls, kind: None = None, default: Optional[TList] = None
    ) -> ContainerList[Any]: ...
    @overload
    def __new__(
        cls, kind: Type[T], default: Optional[TList[T]] = None
    ) -> ContainerList[T]: ...
    @overload
    def __new__(
        cls, kind: Tuple[Type[T]], default: Optional[TList[T]] = None
    ) -> ContainerList[T]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1]],
        default: Optional[TList[Union[T, T1]]] = None,
    ) -> ContainerList[Union[T, T1]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        default: Optional[TList[Union[T, T1, T2]]] = None,
    ) -> ContainerList[Union[T, T1, T2]]: ...
    @overload
    def __new__(
        cls, kind: Member[T, Any], default: Optional[TList[T]] = None
    ) -> ContainerList[T]: ...
