# --------------------------------------------------------------------------------------
# Copyright (c) 2021-2023, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
from typing import Any, Optional, Tuple as TTuple, Type, TypeVar, Union, overload

from .catom import Member

T = TypeVar("T")
T1 = TypeVar("T1")
T2 = TypeVar("T2")

class Tuple(Member[TTuple[T, ...], TTuple[T, ...]]):
    @overload
    def __new__(
        cls, kind: None = None, default: Optional[TTuple[Any, ...]] = None
    ) -> Tuple[Any]: ...
    @overload
    def __new__(
        cls, kind: Type[T], default: Optional[TTuple[T, ...]] = None
    ) -> Tuple[T]: ...
    @overload
    def __new__(
        cls, kind: TTuple[Type[T]], default: Optional[TTuple[T, ...]] = None
    ) -> Tuple[T]: ...
    @overload
    def __new__(
        cls,
        kind: TTuple[Type[T], Type[T1]],
        default: Optional[TTuple[Union[T, T1], ...]] = None,
    ) -> Tuple[Union[T, T1]]: ...
    @overload
    def __new__(
        cls,
        kind: TTuple[Type[T], Type[T1], Type[T2]],
        default: Optional[TTuple[Union[T, T1, T2], ...]] = None,
    ) -> Tuple[Union[T, T1, T2]]: ...
    @overload
    def __new__(
        cls, kind: Member[T, Any], default: Optional[TTuple[T]] = None
    ) -> Tuple[T]: ...
