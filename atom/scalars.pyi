# --------------------------------------------------------------------------------------
# Copyright (c) 2021, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
from typing import (
    Any,
    Callable as TCallable,
    Literal,
    NoReturn,
    Optional,
    TypeVar,
    Union,
    overload,
)

from .catom import Member

T = TypeVar("T")
S = TypeVar("S")

class Value(Member[T, T]):
    def __new__(
        cls, default: Any = None, factory: Optional[TCallable[[], Any]] = None
    ) -> Value[Any]: ...

class ReadOnly(Member[T, T]):
    @overload
    def __new__(cls, default: None = None, factory: None = None) -> ReadOnly[Any]: ...
    @overload
    def __new__(cls, default: None, factory: TCallable[[], T]) -> ReadOnly[T]: ...
    @overload
    def __new__(
        cls, default: T, factory: Optional[TCallable[[], Any]] = None
    ) -> ReadOnly[T]: ...
    @overload
    def __new__(
        cls, default: None = None, *, factory: TCallable[[], T]
    ) -> ReadOnly[T]: ...

class Constant(Member[T, NoReturn]):  # FIXME over-write set del ?
    @overload
    def __new__(cls, default: None = None, factory: None = None) -> Constant[Any]: ...
    @overload
    def __new__(cls, default: None, factory: TCallable[[], T]) -> Constant[T]: ...
    @overload
    def __new__(
        cls, default: None = None, *, factory: TCallable[[], T]
    ) -> Constant[T]: ...
    @overload
    def __new__(
        cls, default: T, factory: Optional[TCallable[[], Any]] = None
    ) -> Constant[T]: ...

C = TypeVar("C", bound=TCallable)

class Callable(Member[T, T]):
    @overload
    def __new__(self, default: None, factory: TCallable[[], C]) -> Callable[C]: ...
    @overload
    def __new__(
        self, default: None = None, *, factory: TCallable[[], C]
    ) -> Callable[C]: ...
    @overload
    def __new__(
        self, default: C, factory: Optional[TCallable[[], TCallable]] = None
    ) -> Callable[C]: ...
    @overload
    def __new__(
        self,
        default: None = None,
        factory: None = None,
    ) -> Callable[TCallable]: ...

class Bool(Member[bool, T]):
    def __new__(
        self, default: bool = False, factory: Optional[TCallable[[], bool]] = None
    ) -> Bool[bool]: ...

class Int(Member[int, T]):
    @overload
    def __new__(
        cls,
        default: int = 0,
        factory: Optional[TCallable[[], int]] = None,
        strict: Literal[True] = True,
    ) -> Int[int]: ...
    @overload
    def __new__(
        cls,
        default: Union[int, float] = 0,
        factory: Optional[TCallable[[], Union[int, float]]] = None,
        *,
        strict: Literal[False],
    ) -> Int[Union[int, float]]: ...
    @overload
    def __new__(
        cls,
        default: Union[int, float],
        factory: Optional[TCallable[[], Union[int, float]]],
        strict: Literal[False],
    ) -> Int[Union[int, float]]: ...

class FloatRange(Member[float, T]):
    def __new__(
        self,
        low: Optional[float] = None,
        high: Optional[float] = None,
        value: Optional[float] = None,
    ) -> FloatRange[float]: ...

class Range(Member[int, T]):
    def __new__(
        self,
        low: Optional[int] = None,
        high: Optional[int] = None,
        value: Optional[int] = None,
    ) -> Range[int]: ...

class Float(Member[float, T]):
    @overload
    def __new__(
        cls,
        default: float = 0.0,
        factory: TCallable[[], Union[int, float]] = None,
        strict: Literal[False] = False,
    ) -> Float[float]: ...
    @overload
    def __new__(
        cls,
        default: float = 0.0,
        factory: TCallable[[], float] = None,
        *,
        strict: Literal[True],
    ) -> Float[float]: ...  # FIXME we cannot encode that an int will be rejected
    @overload
    def __new__(
        cls,
        default: float,
        factory: TCallable[[], float],
        strict: Literal[True],
    ) -> Float[float]: ...  # FIXME we cannot encode that an int will be rejected

class Bytes(Member[bytes, T]):
    @overload
    def __new__(
        cls,
        default: Union[str, bytes] = b"",
        factory: Optional[TCallable[[], Union[str, bytes]]] = None,
        strict: Literal[False] = False,
    ) -> Bytes[Union[bytes, str]]: ...
    @overload
    def __new__(
        cls,
        default: bytes = b"",
        factory: Optional[TCallable[[], bytes]] = None,
        *,
        strict: Literal[True],
    ) -> Bytes[bytes]: ...
    @overload
    def __new__(
        cls,
        default: bytes,
        factory: Optional[TCallable[[], bytes]],
        strict: Literal[True],
    ) -> Bytes[bytes]: ...

class Str(Member[str, T]):
    @overload
    def __new__(
        cls,
        default: Union[str, bytes] = "",
        factory: Optional[TCallable[[], Union[str, bytes]]] = None,
        strict: Literal[False] = False,
    ) -> Str[Union[str, bytes]]: ...
    @overload
    def __new__(
        cls,
        default: str = "",
        factory: Optional[TCallable[[], str]] = None,
        *,
        strict: Literal[True],
    ) -> Str[str]: ...
    @overload
    def __new__(
        cls,
        default: str,
        factory: Optional[TCallable[[], str]],
        strict: Literal[True],
    ) -> Str[str]: ...
