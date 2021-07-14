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
    # - type
    @overload
    def __new__(
        cls,
        kind: Type[T],
        args: None = None,
        kwargs: None = None,
        factory: None = None,
        coercer: None = None,
    ) -> Coerced[Optional[T], T]: ...
    @overload
    def __new__(
        cls,
        kind: Type[T],
        args: None = None,
        kwargs: None = None,
        factory: None = None,
        *,
        coercer: Callable[[S], T],
    ) -> Coerced[Optional[T], Union[T, S]]: ...
    @overload
    def __new__(
        cls,
        kind: Type[T],
        args: None,
        kwargs: None,
        factory: None,
        coercer: Callable[[S], T],
    ) -> Coerced[Optional[T], Union[T, S]]: ...
    # - 1-tuple
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T]],
        args: None = None,
        kwargs: None = None,
        factory: None = None,
        coercer: None = None,
    ) -> Coerced[Optional[T], T]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T]],
        args: None = None,
        kwargs: None = None,
        factory: None = None,
        *,
        coercer: Callable[[S], T],
    ) -> Coerced[Optional[T], Union[T, S]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T]],
        args: None,
        kwargs: None,
        factory: None,
        coercer: Callable[[S], T],
    ) -> Coerced[Optional[T], Union[T, S]]: ...
    # - 2-tuple
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1]],
        args: None = None,
        kwargs: None = None,
        factory: None = None,
        coercer: None = None,
    ) -> Coerced[Optional[Union[T, T1]], Union[T, T1]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1]],
        args: None = None,
        kwargs: None = None,
        factory: None = None,
        *,
        coercer: Callable[[S], Union[T, T1]],
    ) -> Coerced[Optional[Union[T, T1]], Union[T, T1, S]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1]],
        args: None,
        kwargs: None,
        factory: None,
        coercer: Callable[[S], Union[T, T1]],
    ) -> Coerced[Optional[Union[T, T1]], Union[T, T1, S]]: ...
    # - 3-tuple
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        args: None = None,
        kwargs: None = None,
        factory: None = None,
        coercer: None = None,
    ) -> Coerced[Optional[Union[T, T1, T2]], Union[T, T1, T2]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        args: None = None,
        kwargs: None = None,
        factory: None = None,
        *,
        coercer: Callable[[S], Union[T, T1, T2]],
    ) -> Coerced[Optional[Union[T, T1, T2]], Union[T, T1, T2, S]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        args: None,
        kwargs: None,
        factory: None,
        coercer: Callable[[S], Union[T, T1, T2]],
    ) -> Coerced[Optional[Union[T, T1, T2]], Union[T, T1, T2, S]]: ...
    # Default with args
    # - type
    @overload
    def __new__(
        cls,
        kind: Type[T],
        args: tuple,
        kwargs: Optional[Dict[str, Any]] = None,
        factory: None = None,
        coercer: None = None,
    ) -> Coerced[T, T]: ...
    @overload
    def __new__(
        cls,
        kind: Type[T],
        args: tuple,
        kwargs: Optional[Dict[str, Any]] = None,
        factory: None = None,
        *,
        coercer: Callable[[S], T],
    ) -> Coerced[T, Union[T, S]]: ...
    @overload
    def __new__(
        cls,
        kind: Type[T],
        args: tuple,
        kwargs: Optional[Dict[str, Any]],
        factory: None,
        coercer: Callable[[S], T],
    ) -> Coerced[T, Union[T, S]]: ...
    # - 1-tuple
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T]],
        args: tuple,
        kwargs: Optional[Dict[str, Any]] = None,
        factory: None = None,
        coercer: None = None,
    ) -> Coerced[T, T]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T]],
        args: tuple,
        kwargs: Optional[Dict[str, Any]] = None,
        factory: None = None,
        *,
        coercer: Callable[[S], T],
    ) -> Coerced[T, Union[T, S]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T]],
        args: tuple,
        kwargs: Optional[Dict[str, Any]],
        factory: None,
        coercer: Callable[[S], T],
    ) -> Coerced[T, Union[T, S]]: ...
    # - 2-tuple
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1]],
        args: tuple,
        kwargs: Optional[Dict[str, Any]] = None,
        factory: None = None,
        coercer: None = None,
    ) -> Coerced[Union[T, T1], Union[T, T1]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1]],
        args: tuple,
        kwargs: Optional[Dict[str, Any]] = None,
        factory: None = None,
        *,
        coercer: Callable[[S], Union[T, T1]],
    ) -> Coerced[Union[T, T1], Union[T, T1, S]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1]],
        args: tuple,
        kwargs: Optional[Dict[str, Any]],
        factory: None,
        coercer: Callable[[S], Union[T, T1]],
    ) -> Coerced[Union[T, T1], Union[T, T1, S]]: ...
    # - 3-tuple
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        args: tuple,
        kwargs: Optional[Dict[str, Any]] = None,
        factory: None = None,
        coercer: None = None,
    ) -> Coerced[Union[T, T1, T2], Union[T, T1, T2]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        args: tuple,
        kwargs: Optional[Dict[str, Any]] = None,
        factory: None = None,
        *,
        coercer: Callable[[S], Union[T, T1, T2]],
    ) -> Coerced[Union[T, T1, T2], Union[T, T1, T2, S]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        args: tuple,
        kwargs: Optional[Dict[str, Any]],
        factory: None,
        coercer: Callable[[S], Union[T, T1, T2]],
    ) -> Coerced[Union[T, T1, T2], Union[T, T1, T2, S]]: ...
    # Default with kwargs
    # - type
    @overload
    def __new__(
        cls,
        kind: Type[T],
        args: None = None,
        *,
        kwargs: Dict[str, Any],
        factory: None = None,
        coercer: None = None,
    ) -> Coerced[T, T]: ...
    @overload
    def __new__(
        cls,
        kind: Type[T],
        args: None = None,
        *,
        kwargs: Dict[str, Any],
        factory: None = None,
        coercer: Callable[[S], T],
    ) -> Coerced[T, Union[T, S]]: ...
    @overload
    def __new__(
        cls,
        kind: Type[T],
        args: None,
        kwargs: Dict[str, Any],
        factory: None = None,
        coercer: None = None,
    ) -> Coerced[T, T]: ...
    @overload
    def __new__(
        cls,
        kind: Type[T],
        args: None,
        kwargs: Dict[str, Any],
        *,
        factory: None = None,
        coercer: Callable[[S], T],
    ) -> Coerced[T, Union[T, S]]: ...
    @overload
    def __new__(
        cls,
        kind: Type[T],
        args: None,
        kwargs: Dict[str, Any],
        factory: None,
        coercer: Callable[[S], T],
    ) -> Coerced[T, Union[T, S]]: ...
    # - 1-tuple
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T]],
        args: None = None,
        *,
        kwargs: Dict[str, Any],
        factory: None,
        coercer: None = None,
    ) -> Coerced[T, T]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T]],
        args: None = None,
        *,
        kwargs: Dict[str, Any],
        factory: None,
        coercer: Callable[[S], T] = None,
    ) -> Coerced[T, Union[T, S]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T]],
        args: None,
        kwargs: Dict[str, Any],
        *,
        factory: None,
        coercer: None = None,
    ) -> Coerced[T, T]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T]],
        args: None,
        kwargs: Dict[str, Any],
        *,
        factory: None,
        coercer: Callable[[S], T],
    ) -> Coerced[T, Union[T, S]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T]],
        args: None,
        kwargs: Dict[str, Any],
        factory: None,
        coercer: Callable[[S], T],
    ) -> Coerced[T, Union[T, S]]: ...
    # - 2-tuple
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1]],
        args: None = None,
        *,
        kwargs: Dict[str, Any],
        factory: None,
        coercer: None = None,
    ) -> Coerced[Union[T, T1], Union[T, T1]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1]],
        args: None = None,
        *,
        kwargs: Dict[str, Any],
        factory: None,
        coercer: Callable[[S], Union[T, T1]],
    ) -> Coerced[Union[T, T1], Union[T, T1, S]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1]],
        args: None = None,
        *,
        kwargs: Dict[str, Any],
        factory: None,
        coercer: Callable[[S], T],
    ) -> Coerced[Union[T, T1], Union[T, T1, S]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1]],
        args: None = None,
        *,
        kwargs: Dict[str, Any],
        factory: None,
        coercer: Callable[[S], T1],
    ) -> Coerced[Union[T, T1], Union[T, T1, S]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1]],
        args: None,
        kwargs: Dict[str, Any],
        *,
        factory: None,
        coercer: None = None,
    ) -> Coerced[Union[T, T1], Union[T, T1]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1]],
        args: None,
        kwargs: Dict[str, Any],
        *,
        factory: None,
        coercer: Callable[[S], Union[T, T1]],
    ) -> Coerced[Union[T, T1], Union[T, T1, S]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1]],
        args: None,
        kwargs: Dict[str, Any],
        *,
        factory: None,
        coercer: Callable[[S], T],
    ) -> Coerced[Union[T, T1], Union[T, T1, S]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1]],
        args: None,
        kwargs: Dict[str, Any],
        *,
        factory: None,
        coercer: Callable[[S], T1],
    ) -> Coerced[Union[T, T1], Union[T, T1, S]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1]],
        args: None,
        kwargs: Dict[str, Any],
        factory: None,
        coercer: Callable[[S], Union[T, T1]],
    ) -> Coerced[Union[T, T1], Union[T, T1, S]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1]],
        args: None,
        kwargs: Dict[str, Any],
        factory: None,
        coercer: Callable[[S], T],
    ) -> Coerced[Union[T, T1], Union[T, T1, S]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1]],
        args: None,
        kwargs: Dict[str, Any],
        factory: None,
        coercer: Callable[[S], T1],
    ) -> Coerced[Union[T, T1], Union[T, T1, S]]: ...
    # - 3-tuple
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        args: None = None,
        *,
        kwargs: Dict[str, Any],
        factory: None,
        coercer: None = None,
    ) -> Coerced[Union[T, T1, T2], Union[T, T1, T2]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        args: None = None,
        *,
        kwargs: Dict[str, Any],
        factory: None,
        coercer: Callable[[S], Union[T, T1, T2]],
    ) -> Coerced[Union[T, T1, T2], Union[T, T1, T2, S]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        args: None = None,
        *,
        kwargs: Dict[str, Any],
        factory: None,
        coercer: Callable[[S], Union[T, T1]],
    ) -> Coerced[Union[T, T1, T2], Union[T, T1, T2, S]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        args: None = None,
        *,
        kwargs: Dict[str, Any],
        factory: None,
        coercer: Callable[[S], Union[T1, T2]],
    ) -> Coerced[Union[T, T1, T2], Union[T, T1, T2, S]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        args: None = None,
        *,
        kwargs: Dict[str, Any],
        factory: None,
        coercer: Callable[[S], Union[T, T2]],
    ) -> Coerced[Union[T, T1, T2], Union[T, T1, T2, S]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        args: None = None,
        *,
        kwargs: Dict[str, Any],
        factory: None,
        coercer: Callable[[S], T],
    ) -> Coerced[Union[T, T1, T2], Union[T, T1, T2, S]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        args: None = None,
        *,
        kwargs: Dict[str, Any],
        factory: None,
        coercer: Callable[[S], T1],
    ) -> Coerced[Union[T, T1, T2], Union[T, T1, T2, S]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        args: None = None,
        *,
        kwargs: Dict[str, Any],
        factory: None,
        coercer: Callable[[S], T2],
    ) -> Coerced[Union[T, T1, T2], Union[T, T1, T2, S]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        args: None,
        kwargs: Dict[str, Any],
        *,
        factory: None,
        coercer: None = None,
    ) -> Coerced[Union[T, T1, T2], Union[T, T1, T2]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        args: None,
        kwargs: Dict[str, Any],
        *,
        factory: None,
        coercer: Callable[[S], Union[T, T1, T2]],
    ) -> Coerced[Union[T, T1, T2], Union[T, T1, T2, S]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        args: None,
        kwargs: Dict[str, Any],
        *,
        factory: None,
        coercer: Callable[[S], Union[T, T1]],
    ) -> Coerced[Union[T, T1, T2], Union[T, T1, T2, S]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        args: None,
        kwargs: Dict[str, Any],
        *,
        factory: None,
        coercer: Callable[[S], Union[T1, T2]],
    ) -> Coerced[Union[T, T1, T2], Union[T, T1, T2, S]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        args: None,
        kwargs: Dict[str, Any],
        *,
        factory: None,
        coercer: Callable[[S], Union[T, T2]],
    ) -> Coerced[Union[T, T1, T2], Union[T, T1, T2, S]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        args: None,
        kwargs: Dict[str, Any],
        *,
        factory: None,
        coercer: Callable[[S], T],
    ) -> Coerced[Union[T, T1, T2], Union[T, T1, T2, S]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        args: None,
        kwargs: Dict[str, Any],
        *,
        factory: None,
        coercer: Callable[[S], T1],
    ) -> Coerced[Union[T, T1, T2], Union[T, T1, T2, S]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        args: None,
        kwargs: Dict[str, Any],
        *,
        factory: None,
        coercer: Callable[[S], T2],
    ) -> Coerced[Union[T, T1, T2], Union[T, T1, T2, S]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        args: None,
        kwargs: Dict[str, Any],
        factory: None,
        coercer: Callable[[S], Union[T, T1, T2]],
    ) -> Coerced[Union[T, T1, T2], Union[T, T1, T2, S]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        args: None,
        kwargs: Dict[str, Any],
        factory: None,
        coercer: Callable[[S], Union[T, T1]],
    ) -> Coerced[Union[T, T1, T2], Union[T, T1, T2, S]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        args: None,
        kwargs: Dict[str, Any],
        factory: None,
        coercer: Callable[[S], Union[T1, T2]],
    ) -> Coerced[Union[T, T1, T2], Union[T, T1, T2, S]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        args: None,
        kwargs: Dict[str, Any],
        factory: None,
        coercer: Callable[[S], Union[T, T2]],
    ) -> Coerced[Union[T, T1, T2], Union[T, T1, T2, S]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        args: None,
        kwargs: Dict[str, Any],
        factory: None,
        coercer: Callable[[S], T],
    ) -> Coerced[Union[T, T1, T2], Union[T, T1, T2, S]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        args: None,
        kwargs: Dict[str, Any],
        factory: None,
        coercer: Callable[[S], T1],
    ) -> Coerced[Union[T, T1, T2], Union[T, T1, T2, S]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        args: None,
        kwargs: Dict[str, Any],
        factory: None,
        coercer: Callable[[S], T2],
    ) -> Coerced[Union[T, T1, T2], Union[T, T1, T2, S]]: ...
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
        factory: Callable[[], T],
        coercer: Callable[[S], T],
    ) -> Coerced[T, Union[T, S]]: ...
    @overload
    def __new__(
        cls,
        kind: Type[T],
        args: None,
        kwargs: None,
        factory: Callable[[], T],
        coercer: None = None,
    ) -> Coerced[T, T]: ...
    @overload
    def __new__(
        cls,
        kind: Type[T],
        args: None,
        kwargs: None,
        factory: Callable[[], T],
        coercer: Callable[[S], T],
    ) -> Coerced[T, Union[T, S]]: ...
    # - 1-tuple
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
        factory: Callable[[], T],
        coercer: Callable[[S], T],
    ) -> Coerced[T, Union[T, S]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T]],
        args: None,
        kwargs: None,
        factory: Callable[[], T],
        coercer: None = None,
    ) -> Coerced[T, T]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T]],
        args: None,
        kwargs: None,
        factory: Callable[[], T],
        coercer: Callable[[S], T],
    ) -> Coerced[T, Union[T, S]]: ...
    # - 2-tuple
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1]],
        args: None = None,
        kwargs: None = None,
        *,
        factory: Callable[[], Union[T, T1]],
        coercer: None = None,
    ) -> Coerced[Union[T, T1], Union[T, T1]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1]],
        args: None = None,
        kwargs: None = None,
        *,
        factory: Callable[[], Union[T, T1]],
        coercer: Callable[[S], Union[T, T1]],
    ) -> Coerced[Union[T, T1], Union[T, T1, S]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1]],
        args: None,
        kwargs: None,
        factory: Callable[[], Union[T, T1]],
        coercer: None = None,
    ) -> Coerced[Union[T, T1], Union[T, T1]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1]],
        args: None,
        kwargs: None,
        factory: Callable[[], Union[T, T1]],
        coercer: Callable[[S], Union[T, T1]],
    ) -> Coerced[Union[T, T1], Union[T, T1, S]]: ...
    # - 3-tuple
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        args: None = None,
        kwargs: None = None,
        *,
        factory: Callable[[], Union[T, T1, T2]],
        coercer: None = None,
    ) -> Coerced[Union[T, T1, T2], Union[T, T1, T2]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        args: None = None,
        kwargs: None = None,
        *,
        factory: Callable[[], Union[T, T1, T2]],
        coercer: Callable[[S], Union[T, T1, T2]],
    ) -> Coerced[Union[T, T1, T2], Union[T, T1, T2, S]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        args: None,
        kwargs: None,
        factory: Callable[[], Union[T, T1, T2]],
        coercer: None = None,
    ) -> Coerced[Union[T, T1, T2], Union[T, T1, T2]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        args: None,
        kwargs: None,
        factory: Callable[[], Union[T, T1, T2]],
        coercer: Callable[[S], Union[T, T1, T2]],
    ) -> Coerced[Union[T, T1, T2], Union[T, T1, T2, S]]: ...
