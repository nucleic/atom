# --------------------------------------------------------------------------------------
# Copyright (c) 2021-2023, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
from typing import (
    Any,
    Callable,
    Dict,
    Literal,
    Optional,
    Tuple,
    Type,
    TypeVar,
    overload,
)

from .catom import Member

T = TypeVar("T")

class Typed(Member[T, T]):
    @overload
    def __new__(
        cls,
        kind: Type[T],
        args: None = None,
        kwargs: None = None,
        *,
        factory: None = None,
        optional: None = None,
    ) -> Typed[Optional[T]]: ...
    @overload
    def __new__(
        cls,
        kind: Type[T],
        args: Tuple[Any, ...],
        kwargs: Optional[Dict[str, Any]] = None,
        *,
        factory: None = None,
        optional: None = None,
    ) -> Typed[T]: ...
    @overload
    def __new__(
        cls,
        kind: Type[T],
        args: None = None,
        *,
        kwargs: Dict[str, Any],
        factory: None = None,
        optional: None = None,
    ) -> Typed[T]: ...
    @overload
    def __new__(
        cls,
        kind: Type[T],
        args: None = None,
        kwargs: None = None,
        *,
        factory: Callable[[], T],
        optional: None = None,
    ) -> Typed[T]: ...
    @overload
    def __new__(
        cls,
        kind: Type[T],
        args: Optional[Tuple[Any, ...]] = None,
        kwargs: Optional[Dict[str, Any]] = None,
        *,
        factory: Optional[Callable[[], T]] = None,
        optional: Literal[True],
    ) -> Typed[Optional[T]]: ...
    @overload
    def __new__(
        cls,
        kind: Type[T],
        args: Optional[Tuple[Any, ...]] = None,
        kwargs: Optional[Dict[str, Any]] = None,
        *,
        factory: Optional[Callable[[], T]] = None,
        optional: Literal[False],
    ) -> Typed[T]: ...

class ForwardTyped(Member[T, T]):
    @overload
    def __new__(
        cls,
        kind: Callable[[], Type[T]],
        args: None = None,
        kwargs: None = None,
        *,
        factory: None = None,
        optional: None = None,
    ) -> ForwardTyped[Optional[T]]: ...
    @overload
    def __new__(
        cls,
        kind: Callable[[], Type[T]],
        args: Tuple[Any, ...],
        kwargs: Optional[Dict[str, Any]] = None,
        *,
        factory: None = None,
        optional: None = None,
    ) -> ForwardTyped[T]: ...
    @overload
    def __new__(
        cls,
        kind: Callable[[], Type[T]],
        args: None = None,
        *,
        kwargs: Dict[str, Any],
        factory: None = None,
        optional: None = None,
    ) -> ForwardTyped[T]: ...
    @overload
    def __new__(
        cls,
        kind: Callable[[], Type[T]],
        args: None = None,
        kwargs: None = None,
        *,
        factory: Callable[[], T],
        optional: None = None,
    ) -> ForwardTyped[T]: ...
    @overload
    def __new__(
        cls,
        kind: Callable[[], Type[T]],
        args: Optional[Tuple[Any, ...]] = None,
        kwargs: Optional[Dict[str, Any]] = None,
        *,
        factory: Optional[Callable[[], T]] = None,
        optional: Literal[True],
    ) -> ForwardTyped[Optional[T]]: ...
    @overload
    def __new__(
        cls,
        kind: Callable[[], Type[T]],
        args: Optional[Tuple[Any, ...]] = None,
        kwargs: Optional[Dict[str, Any]] = None,
        *,
        factory: Optional[Callable[[], T]] = None,
        optional: Literal[False],
    ) -> ForwardTyped[T]: ...
