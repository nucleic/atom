# --------------------------------------------------------------------------------------
# Copyright (c) 2013-2021, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
from typing import Any, Tuple, TypeVar, Union, overload

from .catom import Member

# We cannot type properly an enum, as it would require some dynamic literal so this is
# just a best effort.

T = TypeVar("T")
T1 = TypeVar("T1")

class Enum(Member[T, T]):
    @overload
    def __new__(cls, item: T) -> Enum[T]: ...  # This is hacky but should do
    @overload
    def __new__(cls, *items: T) -> Enum[T]: ...
    @property
    def items(self) -> Tuple[T, ...]: ...
    def added(self: Enum[T], *items: T1) -> Enum[Union[T, T1]]: ...
    def removed(self: Enum[T], *items: Any) -> Enum[T]: ...
    def __call__(self: Enum[T], item: T1) -> Enum[Union[T, T1]]: ...
