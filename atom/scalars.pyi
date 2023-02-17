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
    Tuple,
    Type,
    TypeVar,
    Union,
    overload,
)

from .catom import Member

T = TypeVar("T")
T1 = TypeVar("T1")
T2 = TypeVar("T2")
S = TypeVar("S")

class Value(Member[T, T]):
    def __new__(
        cls, default: Any = None, *, factory: Optional[TCallable[[], Any]] = None
    ) -> Value[Any]: ...

class ReadOnly(Member[T, T]):
    @overload
    def __new__(
        cls, kind: None = None, *, default: None = None, factory: None = None
    ) -> ReadOnly[Any]: ...
    @overload
    def __new__(
        cls, kind: None = None, *, default: T, factory: None = None
    ) -> ReadOnly[T]: ...
    @overload
    def __new__(
        cls, kind: None = None, *, default: None = None, factory: TCallable[[], T]
    ) -> ReadOnly[T]: ...
    @overload
    def __new__(
        cls,
        kind: Type[T],
        *,
        default: Optional[T] = None,
        factory: Optional[TCallable[[], T]] = None,
    ) -> ReadOnly[T]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T]],
        *,
        default: Optional[T] = None,
        factory: Optional[TCallable[[], T]] = None,
    ) -> ReadOnly[T]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1]],
        *,
        default: Optional[T | T1] = None,
        factory: Optional[TCallable[[], T | T1]] = None,
    ) -> ReadOnly[T | T1]: ...
    @overload
    def __new__(
        cls,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
        *,
        default: Optional[T | T1 | T2] = None,
        factory: Optional[TCallable[[], T | T1 | T2]] = None,
    ) -> ReadOnly[T | T1 | T2]: ...

class Constant(Member[T, NoReturn]):  # FIXME over-write del ?
    @overload
    def __new__(
        cls, default: None = None, *, factory: None = None, kind: None = None
    ) -> Constant[Any]: ...
    @overload
    def __new__(
        cls, default: None = None, *, factory: TCallable[[], T], kind: None = None
    ) -> Constant[T]: ...
    @overload
    def __new__(
        cls, default: T, *, factory: None = None, kind: None = None
    ) -> Constant[T]: ...
    @overload
    def __new__(
        cls,
        default: Optional[T] = None,
        *,
        factory: Optional[TCallable[[], T]] = None,
        kind: Type[T],
    ) -> Constant[T]: ...
    @overload
    def __new__(
        cls,
        default: Optional[T] = None,
        *,
        factory: Optional[TCallable[[], T]] = None,
        kind: Tuple[Type[T]],
    ) -> Constant[T]: ...
    @overload
    def __new__(
        cls,
        default: Optional[T | T1] = None,
        *,
        factory: Optional[TCallable[[], T | T1]] = None,
        kind: Tuple[Type[T], Type[T1]],
    ) -> Constant[T | T1]: ...
    @overload
    def __new__(
        cls,
        default: Optional[T | T1 | T2] = None,
        *,
        factory: Optional[TCallable[[], T | T1 | T2]] = None,
        kind: Tuple[Type[T], Type[T1], Type[T2]],
    ) -> Constant[T | T1 | T2]: ...

C = TypeVar("C", bound=TCallable[..., Any])

class Callable(Member[T, T]):
    @overload
    def __new__(
        cls, default: None = None, *, factory: TCallable[[], C]
    ) -> Callable[C]: ...
    @overload
    def __new__(cls, default: C, *, factory: None = None) -> Callable[C]: ...
    @overload
    def __new__(
        cls, default: None = None, *, factory: None = None
    ) -> Callable[TCallable[..., Any]]: ...

class Bool(Member[bool, T]):
    def __new__(
        cls, default: bool = False, *, factory: Optional[TCallable[[], bool]] = None
    ) -> Bool[bool]: ...

class Int(Member[int, T]):
    @overload
    def __new__(
        cls,
        default: int = 0,
        *,
        factory: Optional[TCallable[[], int]] = None,
        strict: Literal[True] = True,
    ) -> Int[int]: ...
    @overload
    def __new__(
        cls,
        default: Union[int, float] = 0,
        *,
        factory: Optional[TCallable[[], Union[int, float]]] = None,
        strict: Literal[False],
    ) -> Int[Union[int, float]]: ...

# NOTE this cannot be properly statically typed checked since Mypy will always accept
# an int where a float is expected
class FloatRange(Member[float, T]):
    @overload
    def __new__(
        cls,
        low: Optional[float] = None,
        high: Optional[float] = None,
        value: Optional[float] = None,
        *,
        strict: Literal[False] = False,
    ) -> FloatRange[int | float]: ...
    @overload
    def __new__(
        cls,
        low: Optional[float] = None,
        high: Optional[float] = None,
        value: Optional[float] = None,
        *,
        strict: Literal[True],
    ) -> FloatRange[float]: ...

class Range(Member[int, T]):
    def __new__(
        cls,
        low: Optional[int] = None,
        high: Optional[int] = None,
        value: Optional[int] = None,
    ) -> Range[int]: ...

class Float(Member[float, T]):
    @overload
    def __new__(
        cls,
        default: float = 0.0,
        *,
        factory: Optional[TCallable[[], Union[int, float]]] = None,
        strict: Literal[False] = False,
    ) -> Float[Union[int, float]]: ...
    @overload
    def __new__(
        cls,
        default: float = 0.0,
        *,
        factory: Optional[TCallable[[], float]] = None,
        strict: Literal[True],
    ) -> Float[float]: ...  # FIXME we cannot encode that an int will be rejected

class Bytes(Member[bytes, T]):
    @overload
    def __new__(
        cls,
        default: bytes = b"",
        *,
        factory: Optional[TCallable[[], bytes]] = None,
        strict: Literal[True] = True,
    ) -> Bytes[bytes]: ...
    @overload
    def __new__(
        cls,
        default: Union[str, bytes] = b"",
        *,
        factory: Optional[TCallable[[], Union[str, bytes]]] = None,
        strict: Literal[False],
    ) -> Bytes[Union[bytes, str]]: ...

class Str(Member[str, T]):
    @overload
    def __new__(
        cls,
        default: str = "",
        *,
        factory: Optional[TCallable[[], str]] = None,
        strict: Literal[True] = True,
    ) -> Str[str]: ...
    @overload
    def __new__(
        cls,
        default: Union[str, bytes] = "",
        *,
        factory: Optional[TCallable[[], Union[str, bytes]]] = None,
        strict: Literal[False],
    ) -> Str[Union[str, bytes]]: ...
