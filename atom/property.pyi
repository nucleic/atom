# --------------------------------------------------------------------------------------
# Copyright (c) 2021, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
from typing import Callable, Optional, TypeVar, NoReturn

from .atom import Atom
from .catom import Member

T = TypeVar("T")
S = TypeVar("S")

class Property(Member):
    def __new__(
        self,
        fget: Optional[Callable[[Atom], T]] = None,
        fset: Optional[Callable[[Atom, S], None]] = None,
        fdel: Optional[Callable[[Atom], None]] = None,
        cached=False,
    ) -> Property[T, S]: ...
    @property
    def fget(self) -> Optional[Callable[[Atom], T]]: ...
    @property
    def fset(self) -> Optional[Callable[[Atom, S], None]]: ...
    @property
    def fdel(self) -> Optional[Callable[[Atom], None]]: ...
    @property
    def cached(self) -> bool: ...
    def getter(self, func: Callable[[Atom], T]) -> Callable[[Atom], T]: ...
    def setter(self, func: Callable[[Atom, S], None]) -> Callable[[Atom, S], None]: ...
    def deleter(self, func: Callable[[Atom], None]) -> Callable[[Atom], None]: ...
    def reset(self, owner: Atom) -> None: ...

def cached_property(fget: Callable[[Atom], T]) -> Property[T, NoReturn]: ...
