# ------------------------------------------------------------------------------
# Copyright (c) 2021, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# ------------------------------------------------------------------------------
import sys
from itertools import chain
from typing import Any, _GenericAlias, TypeVar, get_origin, get_args, Union, Tuple

GENERICS: Tuple[Any, ...] = (_GenericAlias,)
UNION = ()

if sys.version_info >= (3, 9):
    from typing import GenericAlias

    GENERICS += (GenericAlias,)
if sys.version_info >= (3, 10):
    from types import UnionType

    UNION += (UnionType,)


def _extract_types(kind) -> Tuple[type, ...]:
    """Extract a tuple of types from a type-like object"""
    if isinstance(kind, GENERICS):
        kind = get_origin(kind)
        args = get_args(kind)
        if kind is Union:
            ret = list(args)
        else:
            ret = [kind]
    elif UNION and isinstance(kind, UNION):
        ret = list(get_args(kind))
    else:
        ret = [kind]

    for i, t in enumerate(ret):
        if isinstance(t, TypeVar):
            if b := t.__bound__:
                if isinstance(b, str):
                    raise  # XXX
                ret[i] = b
            elif t.__constraints__:
                raise  # XXX

    return tuple(ret)


def extract_types(kind) -> Tuple[type, ...]:
    """Extract a tuple of types from a type-like object or tuple."""
    return tuple(
        chain(
            *[_extract_types(k) for k in (kind if isinstance(kind, tuple) else (kind,))]
        )
    )
