# --------------------------------------------------------------------------------------
# Copyright (c) 2021-2022, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
import sys
from itertools import chain
from typing import (
    TYPE_CHECKING,
    Any,
    List,
    Literal,
    Sequence,
    Tuple,
    TypedDict,
    TypeVar,
    Union,
    get_args,
    get_origin,
)

# In Python 3.9+, List is a _SpecialGenericAlias and does not inherit from
# _GenericAlias which is the type of List[int] for example
GENERICS: Tuple[Any, ...] = (type(List), type(List[int]))
UNION: Tuple[Any, ...] = ()

if sys.version_info >= (3, 9):
    from types import GenericAlias

    GENERICS += (GenericAlias,)
if sys.version_info >= (3, 10):
    from types import UnionType

    UNION += (UnionType,)

if TYPE_CHECKING:
    from .atom import Atom


class _ChangeDict(TypedDict):
    type: Union[
        Literal["create"],
        Literal["update"],
        Literal["delete"],
        Literal["event"],
        Literal["property"],
        Literal["container"],
    ]
    name: str
    object: "Atom"
    value: Any


class ChangeDict(_ChangeDict, total=False):
    oldvalue: Any

    # ContainerList specific entries, present only when type == "container"
    operation: Union[
        Literal["reverse"],
        Literal["__delitem__"],
        Literal["__iadd__"],
        Literal["__imul__"],
        Literal["__setitem__"],
        Literal["append"],
        Literal["extend"],
        Literal["insert"],
        Literal["pop"],
        Literal["remove"],
        Literal["sort"],
    ]
    # The following are present based on the operation value
    olditem: Any  # operation in ("__setitem__",)
    newitem: Any  # operation in ("__setitem__",)
    item: Any  # operation in ("append", "insert", "pop", "remove", "__delitem__")
    index: int  # operation in ("insert", "pop")
    items: Sequence  # operation in ("extend", "__iadd__")
    count: int  # operation in ("__imul__")
    key: Any  # operation in ("sort")
    reverse: bool  # operation in ("sort")


def _extract_types(kind) -> Tuple[type, ...]:
    """Extract a tuple of types from a type-like object"""
    if isinstance(kind, str):
        raise TypeError(
            f"Str-based annotations ({kind!r}) are not supported in atom Members."
        )

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
