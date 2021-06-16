# --------------------------------------------------------------------------------------
# Copyright (c) 2021, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
from typing import List as TList, Tuple, Type, TypeVar, Union, overload

from .catom import Member, atomlist

T = TypeVar("T")
T1 = TypeVar("T1")
T2 = TypeVar("T2")

class List(Member[atomlist[T], atomlist[T]]):
    @overload
    def __new__(cls, kind: Type[T], default: TList[T]) -> List[T]: ...
    @overload
    def __new__(cls, kind: Tuple[Type[T]], default: TList[T]) -> List[T]: ...
    @overload
    def __new__(
        cls, kind: Tuple[Type[T], Type[T1]], default: TList[Union[T, T1]]
    ) -> List[Union[T, T1]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        default: TList[Union[T, T1, T2]],
    ) -> List[Union[T, T1, T2]]: ...
    @overload
    def __new__(cls, kind: Member[T, _], default: TList[T]) -> List[T]: ...
