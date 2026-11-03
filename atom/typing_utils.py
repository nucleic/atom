# --------------------------------------------------------------------------------------
# Copyright (c) 2021-2025, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
import sys
import typing
from itertools import chain
from types import GenericAlias, UnionType
from typing import (
    TYPE_CHECKING,
    Any,
    List,
    Literal,
    NewType,
    Sequence,
    Tuple,
    TypedDict,
    TypeVar,
    Union,
    get_args,
    get_origin,
)

try:
    import typing_extensions
except ImportError:  # pragma: no cover
    typing_extensions = None  # type: ignore[assignment]

_TYPE_ALIAS_TYPES = tuple(
    alias_type
    for alias_type in (
        getattr(typing, "TypeAliasType", None),
        getattr(typing_extensions, "TypeAliasType", None),
    )
    if alias_type is not None
)

if sys.version_info >= (3, 14):
    import annotationlib

    _INVALID_TYPES = (str, annotationlib.ForwardRef)
else:
    _INVALID_TYPES = (str,)

GENERICS = (type(List), type(List[int]), GenericAlias)

UNION = (UnionType,)

TypeLike = Union[type, TypeVar, UnionType, GenericAlias, NewType]

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


def _resolve_type_alias(kind: Any, seen: set[int] | None = None) -> Any:
    """Resolve type aliases to their underlying annotation."""
    if not _TYPE_ALIAS_TYPES or not isinstance(kind, _TYPE_ALIAS_TYPES):
        return kind

    if seen is None:
        seen = set()
    kind_id = id(kind)
    if kind_id in seen:
        raise TypeError(f"Cyclic type alias {kind!r} is not supported.")
    seen.add(kind_id)
    return _resolve_type_alias(kind.__value__, seen)


def _resolve_generic_origin(kind: Any) -> Any:
    """Resolve alias-backed generic origins to their runtime type."""
    seen: set[int] = set()
    current = kind

    while current is not None and id(current) not in seen:
        seen.add(id(current))
        origin = get_origin(current)

        if origin is None:
            value = getattr(current, "__value__", None)
            if value is not None and value is not current:
                current = value
                continue
            return None

        if _TYPE_ALIAS_TYPES and isinstance(origin, _TYPE_ALIAS_TYPES):
            current = origin.__value__
            continue

        if isinstance(origin, type):
            return origin

        value = getattr(origin, "__value__", None)
        if value is not None and value is not origin:
            current = value
            continue

        return None

    return None


def _extract_types(kind: TypeLike) -> Tuple[type, ...]:
    """Extract a tuple of types from a type-like object"""
    if isinstance(kind, _INVALID_TYPES):
        raise TypeError(
            f"Str-based annotations ({kind!r}) are not supported in atom Members."
        )

    ret: List[Any]
    origin = get_origin(kind)
    if origin is Union or (UNION and isinstance(kind, UNION)):
        ret = list(chain.from_iterable(extract_types(a) for a in get_args(kind)))
    else:
        if origin is not None and not isinstance(origin, type):
            origin = _resolve_generic_origin(kind)
        if origin is not None or isinstance(kind, GENERICS):
            ret = [origin or kind]
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
        # NewType only exists for the sake of type checkers so we fall back to
        # the supertype for runtime checks.
        elif isinstance(t, NewType):
            extracted.extend(_extract_types(t.__supertype__))
        elif t is Any:
            extracted.append(object)
        else:
            if not isinstance(t, type):
                raise TypeError(
                    f"Failed to extract types from {kind}. "
                    f"The extraction yielded {t} which is not a type. "
                    "One case in which this can occur is when using unions of "
                    "Literal, and the issues can be worked around by using a "
                    "single literal containing all the values."
                )
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
