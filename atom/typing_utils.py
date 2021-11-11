# ------------------------------------------------------------------------------
# Copyright (c) 2021, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# ------------------------------------------------------------------------------
import sys
from itertools import chain
from typing import Any, TypeVar, Union, Tuple, List


GENERICS: Tuple[Any, ...] = (type(List[int]),)
UNION = ()

if sys.version_info < (3, 8):

    def get_origin(t):
        return t.__origin__

    def get_args(t):
        return t.__args__


else:
    from typing import get_origin, get_args

if sys.version_info >= (3, 9):
    from types import GenericAlias

    GENERICS += (GenericAlias,)
if sys.version_info >= (3, 10):
    from types import UnionType

    UNION += (UnionType,)


def _extract_types(kind) -> Tuple[type, ...]:
    """Extract a tuple of types from a type-like object"""
    if isinstance(kind, GENERICS):
        args = get_args(kind)
        kind = get_origin(kind)
        if kind is Union:
            ret = list(chain.from_iterable(extract_types(a) for a in args))
        else:
            ret = [kind]
    elif UNION and isinstance(kind, UNION):
        ret = list(chain.from_iterable(extract_types(a) for a in get_args(kind)))
    else:
        ret = [kind]

    for i, t in enumerate(ret):
        if isinstance(t, TypeVar):
            b = t.__bound__
            if b:
                if isinstance(b, str):
                    raise ValueError(
                        "Forward reference in type var bounds are not supported."
                    )
                ret[i] = b
            elif t.__constraints__:
                raise ValueError("Constraints in type var are not supported.")

    return tuple(ret)


def extract_types(kind) -> Tuple[type, ...]:
    """Extract a tuple of types from a type-like object or tuple."""
    return tuple(
        chain.from_iterable(
            _extract_types(k) for k in (kind if isinstance(kind, tuple) else (kind,))
        )
    )


NONE_TYPE = type(None)


def is_optional(kinds: Tuple[type, ...]) -> Tuple[bool, Tuple[type, ...]]:
    """Determine if a tuple of types contains NoneType."""
    if NONE_TYPE in kinds:
        return True, tuple(k for k in kinds if k is not NONE_TYPE)
    else:
        return False, kinds
