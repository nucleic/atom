# --------------------------------------------------------------------------------------
# Copyright (c) 2021, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
from typing import Callable, NoReturn, Optional, TypeVar, overload

from .atom import Atom
from .catom import Member

A = TypeVar("A", bound=Atom)
T = TypeVar("T")
S = TypeVar("S")

class Property(Member[T, S]):
    @overload
    def __new__(
        cls,
        fget: None = None,
        fset: None = None,
        fdel: Optional[Callable[[A], None]] = None,
        cached: bool = False,
    ) -> Property[NoReturn, NoReturn]: ...
    @overload
    def __new__(
        cls,
        fget: None,
        fset: Callable[[A, S], None],
        fdel: Optional[Callable[[A], None]] = None,
        cached: bool = False,
    ) -> Property[NoReturn, S]: ...
    @overload
    def __new__(
        cls,
        fget: Callable[[A], T],
        fset: None = None,
        fdel: Optional[Callable[[A], None]] = None,
        cached: bool = False,
    ) -> Property[T, NoReturn]: ...
    @overload
    def __new__(
        cls,
        fget: Callable[[A], T],
        fset: Callable[[A, S], None],
        fdel: Optional[Callable[[A], None]] = None,
        cached: bool = False,
    ) -> Property[T, S]: ...
    @property
    def fget(self) -> Optional[Callable[[Atom], T]]: ...
    @property
    def fset(self) -> Optional[Callable[[Atom, S], None]]: ...
    @property
    def fdel(self) -> Optional[Callable[[Atom], None]]: ...
    @property
    def cached(self) -> bool: ...
    def getter(self, func: Callable[[A], T]) -> Callable[[A], T]: ...
    def setter(self, func: Callable[[A, S], None]) -> Callable[[A, S], None]: ...
    def deleter(self, func: Callable[[A], None]) -> Callable[[A], None]: ...
    def reset(self, owner: Atom) -> None: ...

def cached_property(fget: Callable[[A], T]) -> Property[T, NoReturn]: ...
