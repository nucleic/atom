# --------------------------------------------------------------------------------------
# Copyright (c) 2021-2022, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
import collections.abc
from typing import Any, ClassVar, MutableMapping, Type

from .catom import Member
from .dict import Dict as ADict
from .instance import Instance
from .list import List as AList
from .scalars import Bool, Bytes, Callable as ACallable, Float, Int, Str, Value
from .set import Set as ASet
from .subclass import Subclass
from .typing_utils import extract_types, get_args, is_optional

_NO_DEFAULT = object()

_TYPE_TO_MEMBER = {
    bool: Bool,
    int: Int,
    float: Float,
    str: Str,
    bytes: Bytes,
    list: AList,
    dict: ADict,
    set: ASet,
    collections.abc.Callable: ACallable,
}


def generate_member_from_type_or_generic(
    type_generic: Any, default: Any, annotate_type_containers: int
) -> Member:
    """Generate a member from a type or generic alias."""
    types = extract_types(type_generic)
    parameters = get_args(type_generic)

    m_kwargs = {}

    m_cls: Type[Member]
    if any(
        isinstance(t, type) and issubclass(t, Member) for t in types
    ) and not isinstance(default, Member):
        raise ValueError(
            "Member subclasses cannot be used as annotations without "
            "specifying a default value for the attribute."
        )
    elif object in types or Any in types:
        m_cls = Value
    # Int, Float, Str, Bytes, List, Dict, Set, Tuple, Bool, Callable
    elif len(types) == 1 and types[0] in _TYPE_TO_MEMBER:
        t = types[0]
        m_cls = _TYPE_TO_MEMBER[t]
        if annotate_type_containers and t in (list, dict, set, tuple):
            # We can only validate homogeneous tuple so far so we ignore other cases
            if t is tuple:
                if (...) in parameters or len(set(parameters)) == 1:
                    parameters = (parameters[0],)
                else:
                    parameters = ()
            parameters = tuple(
                generate_member_from_type_or_generic(
                    t, _NO_DEFAULT, annotate_type_containers - 1
                )
                if t not in (Any, object)
                else None
                for t in parameters
            )
        else:
            parameters = ()

    # The value was annotated with Type[T] so we use a subclass
    elif all(t is type for t in types):
        m_cls = Subclass
    else:
        # We cannot determine if a type has a trivial __instancecheck__ or not so
        # we always use Instance since Typed will fail to validate with generic types
        # such as collections.Iterable
        m_cls = Instance

        opt, filtered_types = is_optional(types)
        parameters = (filtered_types,)
        m_kwargs["optional"] = opt
        if opt and default not in (_NO_DEFAULT, None):
            raise ValueError(
                "Members requiring Instance(optional=True) cannot have "
                "a non-None default value."
            )
        elif not opt and default is not _NO_DEFAULT:
            raise ValueError("Members requiring Instance cannot have a default value.")

        # Instance does not have a default keyword so turn a None default into the
        # equivalent no default.
        default = _NO_DEFAULT

    if default is not _NO_DEFAULT:
        m_kwargs["default"] = default

    return m_cls(*parameters, **m_kwargs)


def generate_members_from_cls_namespace(
    cls_name: str, namespace: MutableMapping[str, Any], annotate_type_containers: int
) -> None:
    """Generate the member corresponding to a type annotation."""
    annotations = namespace["__annotations__"]
    from .atom import set_default

    for name, ann in annotations.items():
        default = namespace.get(name, _NO_DEFAULT)

        # We skip field for which a member was already provided or annotations
        # corresponding to class variables.
        if isinstance(default, (Member, set_default)):

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
