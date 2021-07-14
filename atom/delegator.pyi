# --------------------------------------------------------------------------------------
# Copyright (c) 2021, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
from typing import TypeVar

from .catom import Member

T = TypeVar("T")
S = TypeVar("S")

class Delegator(Member[T, S]):
    def __new__(cls, member: Member[T, S]) -> Delegator[T, S]: ...
