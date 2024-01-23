# --------------------------------------------------------------------------------------
# Copyright (c) 2021-2023, Nucleic Development Team.
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
if sys.version_info >= (3, 9):
    from types import GenericAlias

    GENERICS = (type(List), type(List[int]), GenericAlias)
else:
    GENERICS = (type(List), type(List[int]))

if sys.version_info >= (3, 10):
    from types import UnionType

    UNION = (UnionType,)
else:
    UNION = ()

# Type checker consider that typing.List and typing.List[int] are types even though
# there are not at runtime.
if sys.version_info >= (3, 10):
    from types import GenericAlias, UnionType

    TypeLike = Union[type, TypeVar, UnionType, GenericAlias]
elif sys.version_info >= (3, 9):
    from types import GenericAlias

    TypeLike = Union[type, TypeVar, GenericAlias]
else:  # 3.8
    TypeLike = Union[type, TypeVar]

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
    items: Sequence[Any]  # operation in ("extend", "__iadd__")
    count: int  # operation in ("__imul__")
    key: Any  # operation in ("sort")
    reverse: bool  # operation in ("sort")


def _extract_types(kind: TypeLike) -> Tuple[type, ...]:
    """Extract a tuple of types from a type-like object"""
    if isinstance(kind, str):
        raise TypeError(
            f"Str-based annotations ({kind!r}) are not supported in atom Members."
        )

    ret: List[Any]
    if isinstance(kind, GENERICS):
        args = get_args(kind)
        origin = get_origin(kind)
        if origin is Union:
            ret = list(chain.from_iterable(extract_types(a) for a in args))
        else:
            ret = [origin]
    elif UNION and isinstance(kind, UNION):
        ret = list(chain.from_iterable(extract_types(a) for a in get_args(kind)))
    else:
        ret = [kind]

    extracted: List[type] = []
    for t in ret:
        if isinstance(t, TypeVar):
            b = t.__bound__
            if b is not None:
                if isinstance(b, str):
                    raise ValueError(
                        "Forward reference in type var bounds are not supported."
                    )
                extracted.extend(_extract_types(b))
            elif t.__constraints__:
                raise ValueError("Constraints in type var are not supported.")
            else:
                extracted.append(object)

            if t.__contravariant__:
                raise ValueError("TypeVar used in Atom object cannot be contravariant")

        elif t is Any:
            extracted.append(object)
        else:
            assert isinstance(t, type)
            extracted.append(t)

    return tuple(extracted)


def extract_types(kind: Union[TypeLike, Tuple[TypeLike, ...]]) -> Tuple[type, ...]:
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
