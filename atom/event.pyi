# --------------------------------------------------------------------------------------
# Copyright (c) 2021, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
from typing import Any, NoReturn, Type, TypeVar, Union, overload

from .catom import Member

T = TypeVar("T")

class Event(Member[NoReturn, T]):
    @overload
    def __new__(self, kind: None) -> Event[T]: ...
    @overload
    def __new__(self, kind: Type[T]) -> Event[T]: ...
    @overload
    def __new__(self, kind: Member[Any, T]) -> Event[T]: ...
