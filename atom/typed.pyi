# --------------------------------------------------------------------------------------
# Copyright (c) 2021, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
from typing import Any, Callable, Literal, Optional, Type, TypeVar, overload

from .catom import Member

T = TypeVar("T")

class Typed(Member[T, T]):
    @overload
    def __new__(
        cls,
        kind: Type[T],
        args: Optional[tuple] = None,
        kwargs: Optional[dict[str, Any]] = None,
        *,
        factory: Optional[Callable[[], T]] = None,
        optional: Literal[True] = True
    ) -> Typed[Optional[T]]: ...
    @overload
    def __new__(
        cls,
        kind: Type[T],
        args: Optional[tuple] = None,
        kwargs: Optional[dict[str, Any]] = None,
        *,
        factory: Optional[Callable[[], T]] = None,
        optional: Literal[False]
    ) -> Typed[T]: ...

class ForwardTyped(Member[T, T]):
    # No default
    @overload
    def __new__(
        cls,
        kind: Callable[[], Type[T]],
        args: Optional[tuple] = None,
        kwargs: Optional[dict[str, Any]] = None,
        *,
        factory: Optional[Callable[[], T]] = None,
        optional: Literal[True] = True
    ) -> ForwardTyped[Optional[T]]: ...
    @overload
    def __new__(
        cls,
        kind: Callable[[], Type[T]],
        args: Optional[tuple] = None,
        kwargs: Optional[dict[str, Any]] = None,
        *,
        factory: Optional[Callable[[], T]] = None,
        optional: Literal[False]
    ) -> ForwardTyped[T]: ...
