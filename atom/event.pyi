# --------------------------------------------------------------------------------------
# Copyright (c) 2021-2023, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
from typing import Any, Tuple, Type, TypeVar, Union, overload

from .catom import EventBinder, Member

T = TypeVar("T")
T1 = TypeVar("T1")
T2 = TypeVar("T2")

class Event(Member[EventBinder, T]):
    @overload
    def __new__(cls, kind: None = None) -> Event[Any]: ...
    @overload
    def __new__(cls, kind: Type[T]) -> Event[T]: ...
    @overload
    def __new__(cls, kind: Tuple[Type[T]]) -> Event[T]: ...
    @overload
    def __new__(cls, kind: Tuple[Type[T], Type[T1]]) -> Event[Union[T, T1]]: ...
    @overload
    def __new__(
        cls, kind: Tuple[Type[T], Type[T1], Type[T2]]
    ) -> Event[Union[T, T1, T2]]: ...
    @overload
    def __new__(cls, kind: Member[Any, T]) -> Event[T]: ...
