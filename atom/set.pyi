# --------------------------------------------------------------------------------------
# Copyright (c) 2021, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
from typing import Set as TSet, Tuple, Type, TypeVar, Union, overload

from .catom import Member, atomset

T = TypeVar("T")
T1 = TypeVar("T1")
T2 = TypeVar("T2")

class Set(Member[atomset[T]]):
    @overload
    def __new__(cls, kind: Type[T], default: TSet[T]) -> Set[T]: ...
    @overload
    def __new__(cls, kind: Tuple[Type[T]], default: TSet[T]) -> Set[T]: ...
    @overload
    def __new__(
        cls, kind: Tuple[Type[T], Type[T1]], default: TSet[Union[T, T1]]
    ) -> Set[Union[T, T1]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        default: TSet[Union[T, T1, T2]],
    ) -> Set[Union[T, T1, T2]]: ...
    @overload
    def __new__(cls, kind: Member[T], default: TSet[T]) -> Set[T]: ...
