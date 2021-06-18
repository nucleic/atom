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
    NoReturn,
    Optional,
    TypeVar,
    Union,
    overload,
)

from .catom import Member

T = TypeVar("T")
S = TypeVar("S")

class Value(Member[Any, Any]):
    def __new__(
        cls, default: Any = None, factory: Optional[TCallable[[], None]] = None
    ) -> Value: ...

class ReadOnly(Member[T, T]): ...
class Constant(Member[T, NoReturn]): ...  # XXX over-write set del ?

class Callable(Member[TCallable, TCallable]):
    def __init__(
        self,
        default: Optional[TCallable] = None,
        factory: Optional[TCallable[[], TCallable]] = None,
    ) -> None: ...

class Bool(Member[bool, bool]):
    def __init__(
        self, default: bool = False, factory: Optional[TCallable[[], bool]] = None
    ) -> None: ...

class Int(Member):
    @overload
    def __new__(
        cls,
        default: int = 0,
        factory: Optional[TCallable[[], int]] = None,
        strict: True = True,
    ) -> None: ...
    @overload
    def __new__(
        cls,
        default: Union[int, float] = 0,
        factory: Optional[TCallable[[], Union[int, float]]] = None,
        strict: False = True,
    ) -> None: ...

class FloatRange(Member[float, float]):
    def __init__(
        self,
        low: Optional[float] = None,
        high: Optional[float] = None,
        value: Optional[float] = None,
    ) -> None: ...

class Range(Member[int, int]):
    def __init__(
        self,
        low: Optional[int] = None,
        high: Optional[int] = None,
        value: Optional[int] = None,
    ) -> None: ...

class Float(Member):
    @overload
    def __new__(
        cls,
        default: Union[int, float] = 0.0,
        factory: TCallable[[], Union[int, float]] = None,
        strict: False = False,
    ) -> Float[float, Union[int, float]]: ...
    @overload
    def __new__(
        cls,
        default: Optional[float] = 0.0,
        factory: TCallable[[], float] = None,
        strict: True = False,
    ) -> Float[float, float]: ...

class Bytes(Member):
    @overload
    def __new__(
        cls,
        default: Union[str, bytes] = b"",
        factory: TCallable[[], Union[str, bytes]] = None,
        strict: False = False,
    ) -> Bytes[bytes, Union[bytes, str]]: ...
    @overload
    def __new__(
        cls,
        default: bytes = b"",
        factory: TCallable[[], bytes] = None,
        strict: True = False,
    ) -> Bytes[bytes, bytes]: ...

class Str(Member):
    @overload
    def __new__(
        cls,
        default: Union[str, bytes] = "",
        factory: TCallable[[], Union[str, bytes]] = None,
        strict: False = False,
    ) -> Str[str, Union[str, bytes]]: ...
    @overload
    def __new__(
        cls,
        default: str = "",
        factory: TCallable[[], str] = None,
        strict: True = False,
    ) -> Str[str, str]: ...
