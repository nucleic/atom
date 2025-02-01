# --------------------------------------------------------------------------------------
# Copyright (c) 2021-2024, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
import collections.abc
from collections import defaultdict
from typing import Any, ClassVar, Literal, MutableMapping, Type

from ..catom import Member
from ..coerced import Coerced
from ..dict import DefaultDict, Dict as ADict
from ..enum import Enum
from ..instance import Instance
from ..list import List as AList
from ..scalars import Bool, Bytes, Callable as ACallable, Float, Int, Str, Value
from ..set import Set as ASet
from ..subclass import Subclass
from ..tuple import FixedTuple, Tuple as ATuple
from ..typed import Typed
from ..typing_utils import extract_types, get_args, get_origin, is_optional
from .member_modifiers import _SENTINEL, member

_NO_DEFAULT = object()

_TYPE_TO_MEMBER = {
    bool: Bool,
    int: Int,
    float: Float,
    str: Str,
    bytes: Bytes,
    list: AList,
    dict: ADict,
    defaultdict: DefaultDict,
    set: ASet,
    tuple: ATuple,
    collections.abc.Callable: ACallable,
}


def generate_member_from_type_or_generic(
    type_generic: Any, default: Any, annotate_type_containers: int
) -> Member:
    """Generate a member from a type or generic alias."""
    # Here we special case Literal to generate an Enum member.
    types: tuple[type, ...]
    if get_origin(type_generic) is Literal:
        types = ()
    else:
        types = extract_types(type_generic)
    parameters = get_args(type_generic)

    m_kwargs: dict[str, Any] = {}

    m_cls: Type[Member]
    if any(
        isinstance(t, type) and issubclass(t, Member) for t in types
    ) and not isinstance(default, (Member, member)):
        raise ValueError(
            "Member subclasses cannot be used as annotations without "
            "specifying a default value for the attribute."
        )
    elif isinstance(default, member) and default.coercer is not None:
        m_cls = Coerced
        parameters = (types,)
        m_kwargs["coercer"] = default.coercer
    elif object in types or Any in types:
        m_cls = Value
        parameters = ()
    # We are dealing with a Literal, so use an Enum member
    elif not types:
        m_cls = Enum
        if default is not _NO_DEFAULT:
            if default not in parameters:
                raise ValueError(
                    f"Default value {default} does not appear in Literal: {parameters}"
                )
            # Make the default value the first in the enum arguments.
            p = list(parameters)
            p.pop(p.index(default))
            parameters = (default, *p)
            default = _NO_DEFAULT
    # Int, Float, Str, Bytes, List, Dict, Set, Tuple, Bool, Callable
    elif len(types) == 1 and types[0] in _TYPE_TO_MEMBER:
        t = types[0]
        m_cls = _TYPE_TO_MEMBER[t]
        if annotate_type_containers and t in (
            list,
            dict,
            collections.defaultdict,
            set,
            tuple,
        ):
            if t is tuple:
                if (...) in parameters:
                    parameters = (parameters[0],)
                else:
                    m_cls = FixedTuple
            parameters = tuple(
                generate_member_from_type_or_generic(
                    t, _NO_DEFAULT, annotate_type_containers - 1
                )
                if t not in (Any, object)
                else Value()
                for t in parameters
            )
        else:
            parameters = ()

    # The value was annotated with Type[T] so we use a subclass
    elif all(t is type for t in types):
        m_cls = Subclass
        assert len(parameters) == 1
        parameters = extract_types(parameters[0])
    else:
        # Only a metaclass can implement __instancecheck__ so we check for an
        # implementation differing from type.__instancecheck__ and use Instance
        # if we find one and otherwise Typed.
        opt, filtered_types = is_optional(types)
        if (
            len(filtered_types) == 1
            and type(filtered_types[0]).__instancecheck__ is type.__instancecheck__
        ):
            m_cls = Typed
        else:
            m_cls = Instance

        parameters = (filtered_types,)
        m_kwargs["optional"] = opt
        if (
            opt
            and not isinstance(default, member)
            and default not in (_NO_DEFAULT, None)
        ):
            raise ValueError(
                "Members requiring Instance(optional=True) cannot have "
                "a non-None default value."
            )
        elif not opt and not isinstance(default, member) and default is not _NO_DEFAULT:
            raise ValueError("Members requiring Instance cannot have a default value.")

        # Instance does not have a default keyword so turn a None default into the
        # equivalent no default.
        if default is None:
            default = _NO_DEFAULT

    if isinstance(default, member):
        if default.default_value is not _SENTINEL:
            m_kwargs["default"] = default.default_value
        elif default.default_factory is not None:
            m_kwargs["factory"] = default.default_factory
        if default.default_args is not None:
            m_kwargs["args"] = default.default_args
        if default.default_kwargs is not None:
            m_kwargs["kwargs"] = default.default_kwargs

    elif default is not _NO_DEFAULT:
        m_kwargs["default"] = default

    new = m_cls(*parameters, **m_kwargs)
    if isinstance(default, member) and default.metadata:
        new.metadata = default.metadata

    return new


def generate_members_from_cls_namespace(
    cls_name: str, namespace: MutableMapping[str, Any], annotate_type_containers: int
) -> None:
    """Generate the member corresponding to a type annotation."""
    annotations = namespace["__annotations__"]

    for name, ann in annotations.items():
        default = namespace.get(name, _NO_DEFAULT)

        # We skip field for which a member was already provided or annotations
        # corresponding to class variables.
        if isinstance(default, Member):
            # Allow string annotations for members
            if isinstance(ann, str):
                continue

            types = extract_types(ann)
            if len(types) != 1 or not issubclass(types[0], Member):
                raise TypeError(
                    f"Field '{name}' of '{cls_name}' is assigned a Member-like value "
                    "but its annotation is not Member compatible"
                )
            continue

        # We also skip fields annotated as class variables.
        elif getattr(ann, "__origin__", None) is ClassVar:
            continue

        try:
            namespace[name] = generate_member_from_type_or_generic(
                ann, default, annotate_type_containers
            )
        except ValueError as e:
            raise ValueError(
                "Encountered an issue when generating a member for field "
                f"'{name}' of '{cls_name}'."
            ) from e
