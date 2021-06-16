# --------------------------------------------------------------------------------------
# Copyright (c) 2021, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
from typing import Any, Callable, Dict, Optional, Type, TypeVar, overload

from .catom import Member

T = TypeVar("T")

class Typed(Member[T, T]):
    @overload
    def __new__(
        cls,
        kind: Type[T],
        args: None = None,
        kwargs: None = None,
        factory: None = None,
    ) -> Typed[Optional[T]]: ...
    @overload
    def __new__(
        cls,
        kind: Type[T],
        args: tuple,
        kwargs: Optional[Dict[str, Any]] = None,
        factory: None = None,
    ) -> Typed[T]: ...
    @overload
    def __new__(
        cls,
        kind: Type[T],
        args: None = None,
        *,
        kwargs: Dict[str, Any],
        factory: None,
    ) -> Typed[T]: ...
    @overload
    def __new__(
        cls,
        kind: Type[T],
        args: None = None,
        kwargs: None = None,
        *,
        factory: Callable[[], T],
    ) -> Typed[T]: ...

class ForwardTyped(Member[T]):
    @overload
    def __new__(
        cls,
        kind: Callable[[], Type[T]],
        args: None = None,
        kwargs: None = None,
        factory: None = None,
    ) -> ForwardTyped[Optional[T]]: ...
    @overload
    def __new__(
        cls,
        kind: Callable[[], Type[T]],
        args: tuple,
        kwargs: Optional[Dict[str, Any]] = None,
        factory: None = None,
    ) -> ForwardTyped[T]: ...
    @overload
    def __new__(
        cls,
        kind: Callable[[], Type[T]],
        args: tuple,
        kwargs: Dict[str, Any],
        factory: None,
    ) -> ForwardTyped[T]: ...
    @overload
    def __new__(
        cls,
        kind: Callable[[], Type[T]],
        args: None = None,
        *,
        kwargs: Dict[str, Any],
        factory: None,
    ) -> ForwardTyped[T]: ...
    @overload
    def __new__(
        cls,
        kind: Callable[[], Type[T]],
        args: None = None,
        kwargs: None = None,
        *,
        factory: Callable[[], T],
    ) -> ForwardTyped[T]: ...
