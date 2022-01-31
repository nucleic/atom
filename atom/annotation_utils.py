# --------------------------------------------------------------------------------------
# Copyright (c) 2021-2022, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
import collections.abc
from typing import Any, ClassVar, Type

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
    if object in types or Any in types:
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

        opt, parameters = is_optional(types)
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


def generate_members_from_cls_namespace(namespace, annotate_type_containers):
    """Generate the member corresponding to a type annotation."""
    annotations = namespace["__annotations__"]

    for name, ann in annotations.items():
        default = namespace.get(name, _NO_DEFAULT)
        # We skip field for which a member was already provided or annotations
        # corresponding to class variables.
        if isinstance(default, Member) or getattr(ann, "__origin__", None) is ClassVar:
            continue
        namespace[name] = generate_member_from_type_or_generic(
            ann, default, annotate_type_containers
        )
