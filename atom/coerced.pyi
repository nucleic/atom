# --------------------------------------------------------------------------------------
# Copyright (c) 2021, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
from typing import (
    Any,
    Callable,
    Dict,
    Optional,
    Tuple,
    Type,
    TypeVar,
    Union,
    overload,
)

from .atom import Atom
from .catom import Member

T = TypeVar("T")
T1 = TypeVar("T1")
T2 = TypeVar("T2")

S = TypeVar("S")

class Coerced(Member[T, S]):
    # No default
    @overload
    def __new__(
        cls,
        kind: Type[T],
        args: None = None,
        kwargs: None = None,
        factory: None = None,
        coercer: Optional[Callable[[S], T]] = None,
    ) -> Coerced[Optional[T], S]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T]],
        args: None = None,
        kwargs: None = None,
        factory: None = None,
        coercer: Optional[Callable[[S], T]] = None,
    ) -> Coerced[Optional[T], S]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1]],
        args: None = None,
        kwargs: None = None,
        factory: None = None,
        coercer: Optional[Callable[[S], Union[T, T1]]] = None,
    ) -> Coerced[Optional[Union[T, T1]], S]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        args: None = None,
        kwargs: None = None,
        factory: None = None,
        coercer: Optional[Callable[[S], Union[T, T1, T2]]] = None,
    ) -> Coerced[Optional[Union[T, T1, T2]], S]: ...
    # Default with args
    @overload
    def __new__(
        cls,
        kind: Type[T],
        args: tuple,
        kwargs: Optional[Dict[str, Any]] = None,
        factory: None = None,
        coercer: Optional[Callable[[S], T]] = None,
    ) -> Coerced[T, S]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T]],
        args: tuple,
        kwargs: Optional[Dict[str, Any]] = None,
        factory: None = None,
        coercer: Optional[Callable[[S], T]] = None,
    ) -> Coerced[T, S]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1]],
        args: tuple,
        kwargs: Optional[Dict[str, Any]] = None,
        factory: None = None,
        coercer: Optional[Callable[[S], Union[T, T1]]] = None,
    ) -> Coerced[Union[T, T1], S]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        args: tuple,
        kwargs: Optional[Dict[str, Any]] = None,
        factory: None = None,
        coercer: Optional[Callable[[S], Union[T, T1, T2]]] = None,
    ) -> Coerced[Union[T, T1, T2], S]: ...
    # Default with kwargs
    @overload
    def __new__(
        cls,
        kind: Type[T],
        args: None = None,
        *,
        kwargs: Dict[str, Any],
        factory: None,
        coercer: Optional[Callable[[S], T]] = None,
    ) -> Coerced[T, S]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T]],
        args: None = None,
        *,
        kwargs: Dict[str, Any],
        factory: None,
        coercer: Optional[Callable[[S], T]] = None,
    ) -> Coerced[T, S]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1]],
        args: None = None,
        *,
        kwargs: Dict[str, Any],
        factory: None,
        coercer: Optional[Callable[[S], Union[T, T1]]] = None,
    ) -> Coerced[Union[T, T1], S]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        args: None = None,
        *,
        kwargs: Dict[str, Any],
        factory: None,
        coercer: Optional[Callable[[S], Union[T, T1, T2]]] = None,
    ) -> Coerced[Union[T, T1, T2], S]: ...
    # Default with factory
    @overload
    def __new__(
        cls,
        kind: Type[T],
        args: None = None,
        kwargs: None = None,
        *,
        factory: Callable[[], T],
        coercer: Optional[Callable[[S], T]] = None,
    ) -> Coerced[T, S]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T]],
        args: None = None,
        kwargs: None = None,
        *,
        factory: Callable[[], T],
        coercer: Optional[Callable[[S], T]] = None,
    ) -> Coerced[T, S]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1]],
        args: None = None,
        kwargs: None = None,
        *,
        factory: Callable[[], Union[T, T1]],
        coercer: Optional[Callable[[S], Union[T, T1]]] = None,
    ) -> Coerced[Union[T, T1], S]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        args: None = None,
        kwargs: None = None,
        *,
        factory: Callable[[], Union[T, T1, T2]],
        coercer: Optional[Callable[[S], Union[T, T1, T2]]] = None,
    ) -> Coerced[Union[T, T1, T2], S]: ...
    def __set__(self, instance: Atom, value: S) -> None: ...
