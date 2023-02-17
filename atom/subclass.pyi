# --------------------------------------------------------------------------------------
# Copyright (c) 2021-2023, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
from typing import Callable, Tuple, Type, TypeVar, Union, overload

from .catom import Member

T = TypeVar("T")
T1 = TypeVar("T1")
T2 = TypeVar("T2")

class Subclass(Member[T, T]):
    # No default
    @overload
    def __new__(cls, kind: Type[T], default: None = None) -> Subclass[Type[T]]: ...
    @overload
    def __new__(
        cls, kind: Tuple[Type[T]], default: None = None
    ) -> Subclass[Type[T]]: ...
    @overload
    def __new__(
        cls, kind: Tuple[Type[T], Type[T1]], default: None = None
    ) -> Subclass[Union[Type[T], Type[T1]]]: ...
    @overload
    def __new__(
        cls, kind: Tuple[Type[T], Type[T1], Type[T2]], default: None = None
    ) -> Subclass[Union[Type[T], Type[T1], Type[T2]]]: ...
    # With default
    @overload
    def __new__(cls, kind: Type[T], default: Type[T]) -> Subclass[Type[T]]: ...
    @overload
    def __new__(cls, kind: Tuple[Type[T]], default: Type[T]) -> Subclass[Type[T]]: ...
    @overload
    def __new__(
        cls, kind: Tuple[Type[T], Type[T1]], default: Union[Type[T], Type[T1]]
    ) -> Subclass[Union[Type[T], Type[T1]]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        default: Union[Type[T], Type[T1], Type[T2]],
    ) -> Subclass[Union[Type[T], Type[T1], Type[T2]]]: ...

class ForwardSubclass(Subclass[T]):
    @overload
    def __new__(cls, resolve: Callable[[], Type[T]]) -> ForwardSubclass[Type[T]]: ...
    @overload
    def __new__(
        cls, resolve: Callable[[], Tuple[Type[T]]]
    ) -> ForwardSubclass[Type[T]]: ...
    @overload
    def __new__(
        cls, resolve: Callable[[], Tuple[Type[T], Type[T1]]]
    ) -> ForwardSubclass[Union[Type[T], Type[T1]]]: ...
    @overload
    def __new__(
        cls, resolve: Callable[[], Tuple[Type[T], Type[T1], Type[T2]]]
    ) -> ForwardSubclass[Union[Type[T], Type[T1], Type[T2]]]: ...
