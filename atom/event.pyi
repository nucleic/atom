# --------------------------------------------------------------------------------------
# Copyright (c) 2021, Nucleic Development Team.
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
    def __new__(self, kind: None = None) -> Event[Any]: ...
    @overload
    def __new__(self, kind: Type[T]) -> Event[T]: ...
    @overload
    def __new__(self, kind: Tuple[Type[T]]) -> Event[T]: ...
    @overload
    def __new__(self, kind: Tuple[Type[T], Type[T1]]) -> Event[Union[T, T1]]: ...
    @overload
    def __new__(
        self, kind: Tuple[Type[T], Type[T1], Type[T2]]
    ) -> Event[Union[T, T1, T2]]: ...
    @overload
    def __new__(self, kind: Member[Any, T]) -> Event[T]: ...
