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
T1 = TypeVar("T1")
T2 = TypeVar("T2")

class Instance(Member[T, T]):
    # Single Type
    @overload
    def __new__(
        cls,
        kind: Type[T],
        args: None = None,
        kwargs: None = None,
        *,
        factory: None = None,
        optional: None = None,
    ) -> Instance[Optional[T]]: ...
    @overload
    def __new__(
        cls,
        kind: Type[T],
        args: Tuple[Any, ...],
        kwargs: Optional[Dict[str, Any]] = None,
        *,
        factory: None = None,
        optional: None = None,
    ) -> Instance[T]: ...
    @overload
    def __new__(
        cls,
        kind: Type[T],
        args: None = None,
        *,
        kwargs: Dict[str, Any],
        factory: None = None,
        optional: None = None,
    ) -> Instance[T]: ...
    @overload
    def __new__(
        cls,
        kind: Type[T],
        args: None = None,
        kwargs: None = None,
        *,
        factory: Callable[[], T],
        optional: None = None,
    ) -> Instance[T]: ...
    @overload
    def __new__(
        cls,
        kind: Type[T],
        args: Optional[Tuple[Any, ...]] = None,
        kwargs: Optional[Dict[str, Any]] = None,
        *,
        factory: Optional[Callable[[], T]] = None,
        optional: Literal[True],
    ) -> Instance[Optional[T]]: ...
    @overload
    def __new__(
        cls,
        kind: Type[T],
        args: Optional[Tuple[Any, ...]] = None,
        kwargs: Optional[Dict[str, Any]] = None,
        *,
        factory: Optional[Callable[[], T]] = None,
        optional: Literal[False],
    ) -> Instance[T]: ...
    # 1-Tuple[Any, ...]
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T]],
        args: None = None,
        kwargs: None = None,
        *,
        factory: None = None,
        optional: None = None,
    ) -> Instance[Optional[T]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T]],
        args: Tuple[Any, ...],
        kwargs: Optional[Dict[str, Any]] = None,
        *,
        factory: None = None,
        optional: None = None,
    ) -> Instance[T]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T]],
        args: None = None,
        *,
        kwargs: Dict[str, Any],
        factory: None = None,
        optional: None = None,
    ) -> Instance[T]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T]],
        args: None = None,
        kwargs: None = None,
        *,
        factory: Callable[[], T],
        optional: None = None,
    ) -> Instance[T]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T]],
        args: Optional[Tuple[Any, ...]] = None,
        kwargs: Optional[Dict[str, Any]] = None,
        *,
        factory: Optional[Callable[[], T]] = None,
        optional: Literal[True],
    ) -> Instance[Optional[T]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T]],
        args: Optional[Tuple[Any, ...]] = None,
        kwargs: Optional[Dict[str, Any]] = None,
        *,
        factory: Optional[Callable[[], T]] = None,
        optional: Literal[False],
    ) -> Instance[T]: ...
    # 2-Tuple[Any, ...]
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1]],
        args: None = None,
        kwargs: None = None,
        *,
        factory: None = None,
        optional: None = None,
    ) -> Instance[Optional[T | T1]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1]],
        args: Tuple[Any, ...],
        kwargs: Optional[Dict[str, Any]] = None,
        *,
        factory: None = None,
        optional: None = None,
    ) -> Instance[T | T1]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1]],
        args: None = None,
        *,
        kwargs: Dict[str, Any],
        factory: None = None,
        optional: None = None,
    ) -> Instance[T | T1]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1]],
        args: None = None,
        kwargs: None = None,
        *,
        factory: Callable[[], T] | Callable[[], T1] | Callable[[], T | T1],
        optional: None = None,
    ) -> Instance[T | T1]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1]],
        args: Optional[Tuple[Any, ...]] = None,
        kwargs: Optional[Dict[str, Any]] = None,
        *,
        factory: Optional[
            Callable[[], T] | Callable[[], T1] | Callable[[], T | T1]
        ] = None,
        optional: Literal[True],
    ) -> Instance[Optional[T | T1]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1]],
        args: Optional[Tuple[Any, ...]] = None,
        kwargs: Optional[Dict[str, Any]] = None,
        *,
        factory: Optional[
            Callable[[], T] | Callable[[], T1] | Callable[[], T | T1]
        ] = None,
        optional: Literal[False],
    ) -> Instance[T | T1]: ...
    # 3-Tuple[Any, ...]
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        args: None = None,
        kwargs: None = None,
        *,
        factory: None = None,
        optional: None = None,
    ) -> Instance[Optional[T | T1 | T2]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        args: Tuple[Any, ...],
        kwargs: Optional[Dict[str, Any]] = None,
        *,
        factory: None = None,
        optional: None = None,
    ) -> Instance[T | T1 | T2]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        args: None = None,
        *,
        kwargs: Dict[str, Any],
        factory: None = None,
        optional: None = None,
    ) -> Instance[T | T1 | T2]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        args: None = None,
        kwargs: None = None,
        *,
        factory: Callable[[], T]
        | Callable[[], T1]
        | Callable[[], T2]
        | Callable[[], T | T1]
        | Callable[[], T | T2]
        | Callable[[], T1 | T2]
        | Callable[[], T | T1 | T2],
        optional: None = None,
    ) -> Instance[T | T1 | T2]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        args: Optional[Tuple[Any, ...]] = None,
        kwargs: Optional[Dict[str, Any]] = None,
        *,
        factory: Optional[
            Callable[[], T]
            | Callable[[], T1]
            | Callable[[], T2]
            | Callable[[], T | T1]
            | Callable[[], T | T2]
            | Callable[[], T1 | T2]
            | Callable[[], T | T1 | T2]
        ] = None,
        optional: Literal[True],
    ) -> Instance[Optional[T | T1 | T2]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        args: Optional[Tuple[Any, ...]] = None,
        kwargs: Optional[Dict[str, Any]] = None,
        *,
        factory: Optional[
            Callable[[], T]
            | Callable[[], T1]
            | Callable[[], T2]
            | Callable[[], T | T1]
            | Callable[[], T | T2]
            | Callable[[], T1 | T2]
            | Callable[[], T | T1 | T2]
        ] = None,
        optional: Literal[False],
    ) -> Instance[T | T1 | T2]: ...

class ForwardInstance(Member[T, T]):
    # Single Type
    @overload
    def __new__(
        cls,
        kind: Callable[[], Type[T]],
        args: None = None,
        kwargs: None = None,
        *,
        factory: None = None,
        optional: None = None,
    ) -> ForwardInstance[Optional[T]]: ...
    @overload
    def __new__(
        cls,
        kind: Callable[[], Type[T]],
        args: Tuple[Any, ...],
        kwargs: Optional[Dict[str, Any]] = None,
        *,
        factory: None = None,
        optional: None = None,
    ) -> ForwardInstance[T]: ...
    @overload
    def __new__(
        cls,
        kind: Callable[[], Type[T]],
        args: None = None,
        *,
        kwargs: Dict[str, Any],
        factory: None = None,
        optional: None = None,
    ) -> ForwardInstance[T]: ...
    @overload
    def __new__(
        cls,
        kind: Callable[[], Type[T]],
        args: None = None,
        kwargs: None = None,
        *,
        factory: Callable[[], T],
        optional: None = None,
    ) -> ForwardInstance[T]: ...
    @overload
    def __new__(
        cls,
        kind: Callable[[], Type[T]],
        args: Optional[Tuple[Any, ...]] = None,
        kwargs: Optional[Dict[str, Any]] = None,
        *,
        factory: Optional[Callable[[], T]] = None,
        optional: Literal[True],
    ) -> ForwardInstance[Optional[T]]: ...
    @overload
    def __new__(
        cls,
        kind: Callable[[], Type[T]],
        args: Optional[Tuple[Any, ...]] = None,
        kwargs: Optional[Dict[str, Any]] = None,
        *,
        factory: Optional[Callable[[], T]] = None,
        optional: Literal[False],
    ) -> ForwardInstance[T]: ...
    # 1-Tuple[Any, ...]
    @overload
    def __new__(
        cls,
        kind: Callable[[], Tuple[Type[T]]],
        args: None = None,
        kwargs: None = None,
        *,
        factory: None = None,
        optional: None = None,
    ) -> ForwardInstance[Optional[T]]: ...
    @overload
    def __new__(
        cls,
        kind: Callable[[], Tuple[Type[T]]],
        args: Tuple[Any, ...],
        kwargs: Optional[Dict[str, Any]] = None,
        *,
        factory: None = None,
        optional: None = None,
    ) -> ForwardInstance[T]: ...
    @overload
    def __new__(
        cls,
        kind: Callable[[], Tuple[Type[T]]],
        args: None = None,
        *,
        kwargs: Dict[str, Any],
        factory: None = None,
        optional: None = None,
    ) -> ForwardInstance[T]: ...
    @overload
    def __new__(
        cls,
        kind: Callable[[], Tuple[Type[T]]],
        args: None = None,
        kwargs: None = None,
        *,
        factory: Callable[[], T],
        optional: None = None,
    ) -> ForwardInstance[T]: ...
    @overload
    def __new__(
        cls,
        kind: Callable[[], Tuple[Type[T]]],
        args: Optional[Tuple[Any, ...]] = None,
        kwargs: Optional[Dict[str, Any]] = None,
        *,
        factory: Optional[Callable[[], T]] = None,
        optional: Literal[True],
    ) -> ForwardInstance[Optional[T]]: ...
    @overload
    def __new__(
        cls,
        kind: Callable[[], Tuple[Type[T]]],
        args: Optional[Tuple[Any, ...]] = None,
        kwargs: Optional[Dict[str, Any]] = None,
        *,
        factory: Optional[Callable[[], T]] = None,
        optional: Literal[False],
    ) -> ForwardInstance[T]: ...
    # 2-Tuple[Any, ...]
    @overload
    def __new__(
        cls,
        kind: Callable[[], Tuple[Type[T], Type[T1]]],
        args: None = None,
        kwargs: None = None,
        *,
        factory: None = None,
        optional: None = None,
    ) -> ForwardInstance[Optional[T | T1]]: ...
    @overload
    def __new__(
        cls,
        kind: Callable[[], Tuple[Type[T], Type[T1]]],
        args: Tuple[Any, ...],
        kwargs: Optional[Dict[str, Any]] = None,
        *,
        factory: None = None,
        optional: None = None,
    ) -> ForwardInstance[T | T1]: ...
    @overload
    def __new__(
        cls,
        kind: Callable[[], Tuple[Type[T], Type[T1]]],
        args: None = None,
        *,
        kwargs: Dict[str, Any],
        factory: None = None,
        optional: None = None,
    ) -> ForwardInstance[T | T1]: ...
    @overload
    def __new__(
        cls,
        kind: Callable[[], Tuple[Type[T], Type[T1]]],
        args: None = None,
        kwargs: None = None,
        *,
        factory: Callable[[], T] | Callable[[], T1] | Callable[[], T | T1],
        optional: None = None,
    ) -> ForwardInstance[T | T1]: ...
    @overload
    def __new__(
        cls,
        kind: Callable[[], Tuple[Type[T], Type[T1]]],
        args: Optional[Tuple[Any, ...]] = None,
        kwargs: Optional[Dict[str, Any]] = None,
        *,
        factory: Optional[
            Callable[[], T] | Callable[[], T1] | Callable[[], T | T1]
        ] = None,
        optional: Literal[True],
    ) -> ForwardInstance[Optional[T | T1]]: ...
    @overload
    def __new__(
        cls,
        kind: Callable[[], Tuple[Type[T], Type[T1]]],
        args: Optional[Tuple[Any, ...]] = None,
        kwargs: Optional[Dict[str, Any]] = None,
        *,
        factory: Optional[
            Callable[[], T] | Callable[[], T1] | Callable[[], T | T1]
        ] = None,
        optional: Literal[False],
    ) -> ForwardInstance[T | T1]: ...
    # 3-Tuple[Any, ...]
    @overload
    def __new__(
        cls,
        kind: Callable[[], Tuple[Type[T], Type[T1], Type[T2]]],
        args: None = None,
        kwargs: None = None,
        *,
        factory: None = None,
        optional: None = None,
    ) -> ForwardInstance[Optional[T | T1 | T2]]: ...
    @overload
    def __new__(
        cls,
        kind: Callable[[], Tuple[Type[T], Type[T1], Type[T2]]],
        args: Tuple[Any, ...],
        kwargs: Optional[Dict[str, Any]] = None,
        *,
        factory: None = None,
        optional: None = None,
    ) -> ForwardInstance[T | T1 | T2]: ...
    @overload
    def __new__(
        cls,
        kind: Callable[[], Tuple[Type[T], Type[T1], Type[T2]]],
        args: None = None,
        *,
        kwargs: Dict[str, Any],
        factory: None = None,
        optional: None = None,
    ) -> ForwardInstance[T | T1 | T2]: ...
    @overload
    def __new__(
        cls,
        kind: Callable[[], Tuple[Type[T], Type[T1], Type[T2]]],
        args: None = None,
        kwargs: None = None,
        *,
        factory: Callable[[], T]
        | Callable[[], T1]
        | Callable[[], T2]
        | Callable[[], T | T1]
        | Callable[[], T | T2]
        | Callable[[], T1 | T2]
        | Callable[[], T | T1 | T2],
        optional: None = None,
    ) -> ForwardInstance[T | T1 | T2]: ...
    @overload
    def __new__(
        cls,
        kind: Callable[[], Tuple[Type[T], Type[T1], Type[T2]]],
        args: Optional[Tuple[Any, ...]] = None,
        kwargs: Optional[Dict[str, Any]] = None,
        *,
        factory: Optional[
            Callable[[], T]
            | Callable[[], T1]
            | Callable[[], T2]
            | Callable[[], T | T1]
            | Callable[[], T | T2]
            | Callable[[], T1 | T2]
            | Callable[[], T | T1 | T2]
        ] = None,
        optional: Literal[True],
    ) -> ForwardInstance[Optional[T | T1 | T2]]: ...
    @overload
    def __new__(
        cls,
        kind: Callable[[], Tuple[Type[T], Type[T1], Type[T2]]],
        args: Optional[Tuple[Any, ...]] = None,
        kwargs: Optional[Dict[str, Any]] = None,
        *,
        factory: Optional[
            Callable[[], T]
            | Callable[[], T1]
            | Callable[[], T2]
            | Callable[[], T | T1]
            | Callable[[], T | T2]
            | Callable[[], T1 | T2]
            | Callable[[], T | T1 | T2]
        ] = None,
        optional: Literal[False],
    ) -> ForwardInstance[T | T1 | T2]: ...
