# --------------------------------------------------------------------------------------
# Copyright (c) 2021, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
from typing import Any, Callable, Dict, Optional, Tuple, Type, TypeVar, overload

from .catom import Member

T = TypeVar("T")
T1 = TypeVar("T1")
T2 = TypeVar("T2")

S = TypeVar("S")

class Coerced(Member[T, S]):
    # No default
    # - type
    @overload
    def __new__(
        cls,
        kind: Type[T],
        args: Optional[Tuple[Any, ...]] = None,
        kwargs: Optional[Dict[str, Any]] = None,
        *,
        factory: None = None,
        coercer: None = None,
    ) -> Coerced[T, T]: ...
    @overload
    def __new__(
        cls,
        kind: Type[T],
        args: Optional[Tuple[Any, ...]] = None,
        kwargs: Optional[Dict[str, Any]] = None,
        *,
        factory: None = None,
        coercer: Callable[[S], T],
    ) -> Coerced[T, T | S]: ...
    # - 1-Tuple[Any, ...]
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T]],
        args: Optional[Tuple[Any, ...]] = None,
        kwargs: Optional[Dict[str, Any]] = None,
        *,
        factory: None = None,
        coercer: None = None,
    ) -> Coerced[T, T]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T]],
        args: Optional[Tuple[Any, ...]] = None,
        kwargs: Optional[Dict[str, Any]] = None,
        *,
        factory: None = None,
        coercer: Callable[[S], T],
    ) -> Coerced[T, T | S]: ...
    # - 2-Tuple[Any, ...]
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1]],
        args: Optional[Tuple[Any, ...]] = None,
        kwargs: Optional[Dict[str, Any]] = None,
        *,
        factory: None = None,
        coercer: None = None,
    ) -> Coerced[T | T1, T | T1]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1]],
        args: Optional[Tuple[Any, ...]] = None,
        kwargs: Optional[Dict[str, Any]] = None,
        *,
        factory: None = None,
        coercer: Callable[[S], T | T1],
    ) -> Coerced[T | T1, T | T1 | S]: ...
    # - 3-Tuple[Any, ...]
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        args: Optional[Tuple[Any, ...]] = None,
        kwargs: Optional[Dict[str, Any]] = None,
        *,
        factory: None = None,
        coercer: None = None,
    ) -> Coerced[T | T1 | T2, T | T1 | T2]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        args: Optional[Tuple[Any, ...]] = None,
        kwargs: Optional[Dict[str, Any]] = None,
        *,
        factory: None = None,
        coercer: Callable[[S], T | T1 | T2],
    ) -> Coerced[T | T1 | T2, T | T1 | T2 | S]: ...
    # Default with factory
    # - type
    @overload
    def __new__(
        cls,
        kind: Type[T],
        args: None = None,
        kwargs: None = None,
        *,
        factory: Callable[[], T],
        coercer: None = None,
    ) -> Coerced[T, T]: ...
    @overload
    def __new__(
        cls,
        kind: Type[T],
        args: None = None,
        kwargs: None = None,
        *,
        factory: Callable[[], T | S],
        coercer: Callable[[S], T],
    ) -> Coerced[T, T | S]: ...
    # - 1-Tuple[Any, ...]
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T]],
        args: None = None,
        kwargs: None = None,
        *,
        factory: Callable[[], T],
        coercer: None = None,
    ) -> Coerced[T, T]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T]],
        args: None = None,
        kwargs: None = None,
        *,
        factory: Callable[[], T | S],
        coercer: Callable[[S], T],
    ) -> Coerced[T, T | S]: ...
    # - 2-Tuple[Any, ...]
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1]],
        args: None = None,
        kwargs: None = None,
        *,
        factory: Callable[[], T | T1],
        coercer: None = None,
    ) -> Coerced[T | T1, T | T1]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1]],
        args: None = None,
        kwargs: None = None,
        *,
        factory: Callable[[], T | T1 | S],
        coercer: Callable[[S], T | T1],
    ) -> Coerced[T | T1, T | T1 | S]: ...
    # - 3-Tuple[Any, ...]
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        args: None = None,
        kwargs: None = None,
        *,
        factory: Callable[[], T | T1 | T2],
        coercer: None = None,
    ) -> Coerced[T | T1 | T2, T | T1 | T2]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        args: None = None,
        kwargs: None = None,
        *,
        factory: Callable[[], S],
        coercer: Callable[[S], T | T1 | T2 | S],
    ) -> Coerced[T | T1 | T2, T | T1 | T2 | S]: ...
