# --------------------------------------------------------------------------------------
# Copyright (c) 2021, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
from typing import Any, Callable, Dict, Optional, Tuple, Type, TypeVar, Union, overload

from .catom import Member

T = TypeVar("T")
T1 = TypeVar("T1")
T2 = TypeVar("T2")

class Instance(Member[T, T]):
    # No default
    @overload
    def __new__(
        cls,
        kind: Type[T],
        args: None = None,
        kwargs: None = None,
        factory: None = None,
    ) -> Instance[Optional[T]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T]],
        args: None = None,
        kwargs: None = None,
        factory: None = None,
    ) -> Instance[Optional[T]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1]],
        args: None = None,
        kwargs: None = None,
        factory: None = None,
    ) -> Instance[Optional[Union[T, T1]]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        args: None = None,
        kwargs: None = None,
        factory: None = None,
    ) -> Instance[Optional[Union[T, T1, T2]]]: ...
    # Default with args
    @overload
    def __new__(
        cls,
        kind: Type[T],
        args: tuple,
        kwargs: Optional[Dict[str, Any]] = None,
        factory: None = None,
    ) -> Instance[T]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T]],
        args: tuple,
        kwargs: Optional[Dict[str, Any]] = None,
        factory: None = None,
    ) -> Instance[T]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1]],
        args: tuple,
        kwargs: Optional[Dict[str, Any]] = None,
        factory: None = None,
    ) -> Instance[Union[T, T1]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        args: tuple,
        kwargs: Optional[Dict[str, Any]] = None,
        factory: None = None,
    ) -> Instance[Union[T, T1, T2]]: ...
    # Default with kwargs
    @overload
    def __new__(
        cls,
        kind: Type[T],
        args: None = None,
        *,
        kwargs: Dict[str, Any],
        factory: None = None,
    ) -> Instance[T]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T]],
        args: None = None,
        *,
        kwargs: Dict[str, Any],
        factory: None = None,
    ) -> Instance[T]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1]],
        args: None = None,
        *,
        kwargs: Dict[str, Any],
        factory: None = None,
    ) -> Instance[Union[T, T1]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        args: None = None,
        *,
        kwargs: Dict[str, Any],
        factory: None = None,
    ) -> Instance[Union[T, T1, T2]]: ...
    # Default with kwargs as pos
    @overload
    def __new__(
        cls,
        kind: Type[T],
        args: None,
        kwargs: Dict[str, Any],
        factory: None = None,
    ) -> Instance[T]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T]],
        args: None,
        kwargs: Dict[str, Any],
        factory: None = None,
    ) -> Instance[T]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1]],
        args: None,
        kwargs: Dict[str, Any],
        factory: None = None,
    ) -> Instance[Union[T, T1]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        args: None,
        kwargs: Dict[str, Any],
        factory: None = None,
    ) -> Instance[Union[T, T1, T2]]: ...
    # Default with factory
    @overload
    def __new__(
        cls,
        kind: Type[T],
        args: None = None,
        kwargs: None = None,
        *,
        factory: Callable[[], T],
    ) -> Instance[T]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T]],
        args: None = None,
        kwargs: None = None,
        *,
        factory: Callable[[], T],
    ) -> Instance[T]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1]],
        args: None = None,
        kwargs: None = None,
        *,
        factory: Callable[[], Union[T, T1]],
    ) -> Instance[Union[T, T1]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        args: None = None,
        kwargs: None = None,
        *,
        factory: Callable[[], Union[T, T1, T2]],
    ) -> Instance[Union[T, T1, T2]]: ...
    # Default with factory as pos arg
    @overload
    def __new__(
        cls,
        kind: Type[T],
        args: None,
        kwargs: None,
        factory: Callable[[], T],
    ) -> Instance[T]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T]],
        args: None,
        kwargs: None,
        factory: Callable[[], T],
    ) -> Instance[T]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1]],
        args: None,
        kwargs: None,
        factory: Callable[[], Union[T, T1]],
    ) -> Instance[Union[T, T1]]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        args: None,
        kwargs: None,
        factory: Callable[[], Union[T, T1, T2]],
    ) -> Instance[Union[T, T1, T2]]: ...

class ForwardInstance(Member[T, T]):
    # No default
    @overload
    def __new__(
        cls,
        kind: Callable[[], Type[T]],
        args: None = None,
        kwargs: None = None,
        factory: None = None,
    ) -> ForwardInstance[Optional[T]]: ...
    @overload
    def __new__(
        cls,
        kind: Callable[[], Tuple[Type[T]]],
        args: None = None,
        kwargs: None = None,
        factory: None = None,
    ) -> ForwardInstance[Optional[T]]: ...
    @overload
    def __new__(
        cls,
        kind: Callable[[], Tuple[Type[T], Type[T1]]],
        args: None = None,
        kwargs: None = None,
        factory: None = None,
    ) -> ForwardInstance[Optional[Union[T, T1]]]: ...
    @overload
    def __new__(
        cls,
        kind: Callable[[], Tuple[Type[T], Type[T1], Type[T2]]],
        args: None = None,
        kwargs: None = None,
        factory: None = None,
    ) -> ForwardInstance[Optional[Union[T, T1, T2]]]: ...
    # Default with args
    @overload
    def __new__(
        cls,
        kind: Callable[[], Type[T]],
        args: tuple,
        kwargs: Optional[Dict[str, Any]] = None,
        factory: None = None,
    ) -> ForwardInstance[T]: ...
    @overload
    def __new__(
        cls,
        kind: Callable[[], Tuple[Type[T]]],
        args: tuple,
        kwargs: Optional[Dict[str, Any]] = None,
        factory: None = None,
    ) -> ForwardInstance[T]: ...
    @overload
    def __new__(
        cls,
        kind: Callable[[], Tuple[Type[T], Type[T1]]],
        args: tuple,
        kwargs: Optional[Dict[str, Any]] = None,
        factory: None = None,
    ) -> ForwardInstance[Union[T, T1]]: ...
    @overload
    def __new__(
        cls,
        kind: Callable[[], Tuple[Type[T], Type[T1], Type[T2]]],
        args: tuple,
        kwargs: Optional[Dict[str, Any]] = None,
        factory: None = None,
    ) -> ForwardInstance[Union[T, T1, T2]]: ...
    # Default kwargs only
    @overload
    def __new__(
        cls,
        kind: Callable[[], Type[T]],
        args: None = None,
        *,
        kwargs: Dict[str, Any],
        factory: None = None,
    ) -> ForwardInstance[T]: ...
    @overload
    def __new__(
        cls,
        kind: Callable[[], Tuple[Type[T]]],
        args: None = None,
        *,
        kwargs: Dict[str, Any],
        factory: None = None,
    ) -> ForwardInstance[T]: ...
    @overload
    def __new__(
        cls,
        kind: Callable[[], Tuple[Type[T], Type[T1]]],
        args: None = None,
        *,
        kwargs: Dict[str, Any],
        factory: None = None,
    ) -> ForwardInstance[Union[T, T1]]: ...
    @overload
    def __new__(
        cls,
        kind: Callable[[], Tuple[Type[T], Type[T1], Type[T2]]],
        args: None = None,
        *,
        kwargs: Dict[str, Any],
        factory: None = None,
    ) -> ForwardInstance[Union[T, T1, T2]]: ...
    # Default kwargs only as positional
    @overload
    def __new__(
        cls,
        kind: Callable[[], Type[T]],
        args: None,
        kwargs: Dict[str, Any],
        factory: None = None,
    ) -> ForwardInstance[T]: ...
    @overload
    def __new__(
        cls,
        kind: Callable[[], Tuple[Type[T]]],
        args: None,
        kwargs: Dict[str, Any],
        factory: None = None,
    ) -> ForwardInstance[T]: ...
    @overload
    def __new__(
        cls,
        kind: Callable[[], Tuple[Type[T], Type[T1]]],
        args: None,
        kwargs: Dict[str, Any],
        factory: None = None,
    ) -> ForwardInstance[Union[T, T1]]: ...
    @overload
    def __new__(
        cls,
        kind: Callable[[], Tuple[Type[T], Type[T1], Type[T2]]],
        args: None,
        kwargs: Dict[str, Any],
        factory: None = None,
    ) -> ForwardInstance[Union[T, T1, T2]]: ...
    # Default factory as keyword
    @overload
    def __new__(
        cls,
        kind: Callable[[], Type[T]],
        args: None = None,
        kwargs: None = None,
        *,
        factory: Callable[[], T],
    ) -> ForwardInstance[T]: ...
    @overload
    def __new__(
        cls,
        kind: Callable[[], Tuple[Type[T]]],
        args: None = None,
        kwargs: None = None,
        *,
        factory: Callable[[], T],
    ) -> ForwardInstance[T]: ...
    @overload
    def __new__(
        cls,
        kind: Callable[[], Tuple[Type[T], Type[T1]]],
        args: None = None,
        kwargs: None = None,
        *,
        factory: Callable[[], Union[T, T1]],
    ) -> ForwardInstance[Union[T, T1]]: ...
    @overload
    def __new__(
        cls,
        kind: Callable[[], Tuple[Type[T], Type[T1], Type[T2]]],
        args: None = None,
        kwargs: None = None,
        *,
        factory: Callable[[], Union[T, T1, T2]],
    ) -> ForwardInstance[Union[T, T1, T2]]: ...
    # Default factory as pos arg
    @overload
    def __new__(
        cls,
        kind: Callable[[], Type[T]],
        args: None,
        kwargs: None ,
        factory: Callable[[], T],
    ) -> ForwardInstance[T]: ...
    @overload
    def __new__(
        cls,
        kind: Callable[[], Tuple[Type[T]]],
        args: None,
        kwargs: None,
        factory: Callable[[], T],
    ) -> ForwardInstance[T]: ...
    @overload
    def __new__(
        cls,
        kind: Callable[[], Tuple[Type[T], Type[T1]]],
        args: None,
        kwargs: None,
        factory: Callable[[], Union[T, T1]],
    ) -> ForwardInstance[Union[T, T1]]: ...
    @overload
    def __new__(
        cls,
        kind: Callable[[], Tuple[Type[T], Type[T1], Type[T2]]],
        args: None,
        kwargs: None,
        factory: Callable[[], Union[T, T1, T2]],
    ) -> ForwardInstance[Union[T, T1, T2]]: ...
