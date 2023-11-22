# --------------------------------------------------------------------------------------
# Copyright (c) 2021-2023, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
from typing import (
    Any,
    Callable,
    DefaultDict as TDefaultDict,
    Dict as TDict,
    Optional,
    Tuple,
    Type,
    TypeVar,
    overload,
)

from .catom import Member

T = TypeVar("T")

KT = TypeVar("KT")
VT = TypeVar("VT")
KT1 = TypeVar("KT1")
VT1 = TypeVar("VT1")
KT2 = TypeVar("KT2")
VT2 = TypeVar("VT2")

class Dict(Member[TDict[KT, VT], TDict[KT, VT]]):
    # Untyped
    @overload
    def __new__(
        cls,
        key: None = None,
        value: None = None,
        default: Optional[TDict[Any, Any]] = None,
    ) -> Dict[Any, Any]: ...
    # No default
    # Typed keys
    # - type
    @overload
    def __new__(
        cls,
        key: Type[KT],
        value: None = None,
        default: Optional[TDict[Any, Any]] = None,
    ) -> Dict[KT, Any]: ...
    # - 1-tuple
    @overload
    def __new__(
        cls,
        key: Tuple[Type[KT]],
        value: None = None,
        default: Optional[TDict[Any, Any]] = None,
    ) -> Dict[KT, Any]: ...
    # - 2-tuple
    @overload
    def __new__(
        cls,
        key: Tuple[Type[KT], Type[KT1]],
        value: None = None,
        default: Optional[TDict[Any, Any]] = None,
    ) -> Dict[KT | KT1, Any]: ...
    # - 3-tuple
    @overload
    def __new__(
        cls,
        key: Tuple[Type[KT], Type[KT1], Type[KT2]],
        value: None = None,
        default: Optional[TDict[Any, Any]] = None,
    ) -> Dict[KT | KT1 | KT2, Any]: ...
    # - member
    @overload
    def __new__(
        cls,
        key: Member[KT, Any],
        value: None = None,
        default: Optional[TDict[Any, Any]] = None,
    ) -> Dict[KT, Any]: ...
    # Typed values
    # - type
    @overload
    def __new__(
        cls, key: None, value: Type[VT], default: Optional[TDict[Any, Any]] = None
    ) -> Dict[Any, VT]: ...
    # - 1-tuple
    @overload
    def __new__(
        cls,
        key: None,
        value: Tuple[Type[VT]],
        default: Optional[TDict[Any, Any]] = None,
    ) -> Dict[Any, VT]: ...
    # - 2-tuple
    @overload
    def __new__(
        cls,
        key: None,
        value: Tuple[Type[VT], Type[VT1]],
        default: Optional[TDict[Any, Any]] = None,
    ) -> Dict[Any, VT | VT1]: ...
    # - 3-tuple
    @overload
    def __new__(
        cls,
        key: None,
        value: Tuple[Type[VT], Type[VT1], Type[VT2]],
        default: Optional[TDict[Any, Any]] = None,
    ) -> Dict[Any, VT | VT1 | VT2]: ...
    # - member
    @overload
    def __new__(
        cls,
        key: None,
        value: Member[VT, Any],
        default: Optional[TDict[Any, Any]] = None,
    ) -> Dict[Any, VT]: ...
    # Typed value through keyword
    # - type
    @overload
    def __new__(
        cls,
        key: None = None,
        *,
        value: Type[VT],
        default: Optional[TDict[Any, Any]] = None,
    ) -> Dict[Any, VT]: ...
    # - 1-tuple
    @overload
    def __new__(
        cls,
        key: None = None,
        *,
        value: Tuple[Type[VT]],
        default: Optional[TDict[Any, Any]] = None,
    ) -> Dict[Any, VT]: ...
    # - 2-tuple
    @overload
    def __new__(
        cls,
        key: None = None,
        *,
        value: Tuple[Type[VT], Type[VT1]],
        default: Optional[TDict[Any, Any]] = None,
    ) -> Dict[Any, VT | VT1]: ...
    # - 3-tuple
    @overload
    def __new__(
        cls,
        key: None = None,
        *,
        value: Tuple[Type[VT], Type[VT1], Type[VT2]],
        default: Optional[TDict[Any, Any]] = None,
    ) -> Dict[Any, VT | VT1 | VT2]: ...
    # - member
    @overload
    def __new__(
        cls,
        key: None = None,
        *,
        value: Member[VT, Any],
        default: Optional[TDict[Any, Any]] = None,
    ) -> Dict[Any, VT]: ...
    # Typed key and value
    # - value simple type
    #    - key type
    @overload
    def __new__(
        cls, key: Type[KT], value: Type[VT], default: Optional[TDict[Any, Any]] = None
    ) -> Dict[KT, VT]: ...
    #    - key 1-tuple
    @overload
    def __new__(
        cls,
        key: Tuple[Type[KT]],
        value: Type[VT],
        default: Optional[TDict[Any, Any]] = None,
    ) -> Dict[KT, VT]: ...
    #    - key 2-tuple
    @overload
    def __new__(
        cls,
        key: Tuple[Type[KT], Type[KT1]],
        value: Type[VT],
        default: Optional[TDict[Any, Any]] = None,
    ) -> Dict[KT | KT1, VT]: ...
    #    - key 3-tuple
    @overload
    def __new__(
        cls,
        key: Tuple[Type[KT], Type[KT1], Type[KT2]],
        value: Type[VT],
        default: Optional[TDict[Any, Any]] = None,
    ) -> Dict[KT | KT1 | KT2, VT]: ...
    #    - key member
    @overload
    def __new__(
        cls,
        key: Member[KT, Any],
        value: Type[VT],
        default: Optional[TDict[Any, Any]] = None,
    ) -> Dict[KT, VT]: ...
    # - Value as single element tuple
    #    - key type
    @overload
    def __new__(
        cls,
        key: Type[KT],
        value: Tuple[Type[VT]],
        default: Optional[TDict[Any, Any]] = None,
    ) -> Dict[KT, VT]: ...
    #    - key 1-tuple
    @overload
    def __new__(
        cls,
        key: Tuple[Type[KT]],
        value: Tuple[Type[VT]],
        default: Optional[TDict[Any, Any]] = None,
    ) -> Dict[KT, VT]: ...
    #    - key 2-tuple
    @overload
    def __new__(
        cls,
        key: Tuple[Type[KT], Type[KT1]],
        value: Tuple[Type[VT]],
        default: Optional[TDict[Any, Any]] = None,
    ) -> Dict[KT | KT1, VT]: ...
    #    - key 3-tuple
    @overload
    def __new__(
        cls,
        key: Tuple[Type[KT], Type[KT1], Type[KT2]],
        value: Tuple[Type[VT]],
        default: Optional[TDict[Any, Any]] = None,
    ) -> Dict[KT | KT1 | KT2, VT]: ...
    #    - key member
    @overload
    def __new__(
        cls,
        key: Member[KT, Any],
        value: Tuple[Type[VT]],
        default: Optional[TDict[Any, Any]] = None,
    ) -> Dict[KT, VT]: ...
    # - Value as 2-tuple
    #    - key type
    @overload
    def __new__(
        cls,
        key: Type[KT],
        value: Tuple[Type[VT], Type[VT1]],
        default: Optional[TDict[Any, Any]] = None,
    ) -> Dict[KT, VT | VT1]: ...
    #    - key 1-tuple
    @overload
    def __new__(
        cls,
        key: Tuple[Type[KT]],
        value: Tuple[Type[VT], Type[VT1]],
        default: Optional[TDict[Any, Any]] = None,
    ) -> Dict[KT, VT | VT1]: ...
    #    - key 2-tuple
    @overload
    def __new__(
        cls,
        key: Tuple[Type[KT], Type[KT1]],
        value: Tuple[Type[VT], Type[VT1]],
        default: Optional[TDict[Any, Any]] = None,
    ) -> Dict[KT | KT1, VT | VT1]: ...
    #    - key 3-tuple
    @overload
    def __new__(
        cls,
        key: Tuple[Type[KT], Type[KT1], Type[KT2]],
        value: Tuple[Type[VT], Type[VT1]],
        default: Optional[TDict[Any, Any]] = None,
    ) -> Dict[KT | KT1 | KT2, VT | VT1]: ...
    #    - key member
    @overload
    def __new__(
        cls,
        key: Member[KT, Any],
        value: Tuple[Type[VT], Type[VT1]],
        default: Optional[TDict[Any, Any]] = None,
    ) -> Dict[KT, VT | VT1]: ...
    # - Value as 3-tuple
    #   - key type
    @overload
    def __new__(
        cls,
        key: Type[KT],
        value: Tuple[Type[VT], Type[VT1], Type[VT2]],
        default: Optional[TDict[Any, Any]] = None,
    ) -> Dict[KT, VT | VT1 | VT2]: ...
    #   - key 1-tuple
    @overload
    def __new__(
        cls,
        key: Tuple[Type[KT]],
        value: Tuple[Type[VT], Type[VT1], Type[VT2]],
        default: Optional[TDict[Any, Any]] = None,
    ) -> Dict[KT, VT | VT1 | VT2]: ...
    #   - key 2-tuple
    @overload
    def __new__(
        cls,
        key: Tuple[Type[KT], Type[KT1]],
        value: Tuple[Type[VT], Type[VT1], Type[VT2]],
        default: Optional[TDict[Any, Any]] = None,
    ) -> Dict[KT | KT1, VT | VT1 | VT2]: ...
    #   - key 3-tuple
    @overload
    def __new__(
        cls,
        key: Tuple[Type[KT], Type[KT1], Type[KT2]],
        value: Tuple[Type[VT], Type[VT1], Type[VT2]],
        default: Optional[TDict[Any, Any]] = None,
    ) -> Dict[KT | KT1 | KT2, VT | VT1 | VT2]: ...
    #   - key member
    @overload
    def __new__(
        cls,
        key: Member[KT, Any],
        value: Tuple[Type[VT], Type[VT1], Type[VT2]],
        default: Optional[TDict[Any, Any]] = None,
    ) -> Dict[KT, VT | VT1 | VT2]: ...
    # - value as member
    #   - key type
    @overload
    def __new__(
        cls,
        key: Type[KT],
        value: Member[VT, Any],
        default: Optional[TDict[Any, Any]] = None,
    ) -> Dict[KT, VT]: ...
    #   - key 1-tuple
    @overload
    def __new__(
        cls,
        key: Tuple[Type[KT]],
        value: Member[VT, Any],
        default: Optional[TDict[Any, Any]] = None,
    ) -> Dict[KT, VT]: ...
    #   - key 2-tuple
    @overload
    def __new__(
        cls,
        key: Tuple[Type[KT], Type[KT1]],
        value: Member[VT, Any],
        default: Optional[TDict[Any, Any]] = None,
    ) -> Dict[KT | KT1, VT]: ...
    #   - key 3-tuple
    @overload
    def __new__(
        cls,
        key: Tuple[Type[KT], Type[KT1], Type[KT2]],
        value: Member[VT, VT],
        default: Optional[TDict[Any, Any]] = None,
    ) -> Dict[KT | KT1 | KT2, VT]: ...
    #   - key member
    @overload
    def __new__(
        cls,
        key: Member[KT, KT],
        value: Member[VT, VT],
        default: Optional[TDict[Any, Any]] = None,
    ) -> Dict[KT, VT]: ...

class DefaultDict(Member[TDefaultDict[KT, VT], TDefaultDict[KT, VT]]):
    # Untyped
    @overload
    def __new__(
        cls,
        key: None = None,
        value: None = None,
        default: Optional[TDict[Any, Any]] = None,
        *,
        missing: None = None,
    ) -> DefaultDict[Any, Any]: ...
    # Typed by missing
    @overload
    def __new__(
        cls,
        key: None = None,
        value: None = None,
        default: Optional[TDict[Any, Any]] = None,
        *,
        missing: Callable[[], VT],
    ) -> DefaultDict[Any, VT]: ...
    # Typed by defaultdict default value
    @overload
    def __new__(
        cls,
        key: None = None,
        value: None = None,
        *,
        default: TDefaultDict[Any, VT],
        missing: Callable[[], VT],
    ) -> DefaultDict[Any, VT]: ...
    # No default
    # Typed keys
    # - type
    @overload
    def __new__(
        cls,
        key: Type[KT],
        value: None = None,
        default: Optional[TDict[Any, Any]] = None,
        *,
        missing: None = None,
    ) -> DefaultDict[KT, Any]: ...
    # - 1-tuple
    @overload
    def __new__(
        cls,
        key: Tuple[Type[KT]],
        value: None = None,
        default: Optional[TDict[Any, Any]] = None,
    ) -> DefaultDict[KT, Any]: ...
    # - 2-tuple
    @overload
    def __new__(
        cls,
        key: Tuple[Type[KT], Type[KT1]],
        value: None = None,
        default: Optional[TDict[Any, Any]] = None,
    ) -> DefaultDict[KT | KT1, Any]: ...
    # - 3-tuple
    @overload
    def __new__(
        cls,
        key: Tuple[Type[KT], Type[KT1], Type[KT2]],
        value: None = None,
        default: Optional[TDict[Any, Any]] = None,
    ) -> DefaultDict[KT | KT1 | KT2, Any]: ...
    # - member
    @overload
    def __new__(
        cls,
        key: Member[KT, Any],
        value: None = None,
        default: Optional[TDict[Any, Any]] = None,
    ) -> DefaultDict[KT, Any]: ...
    # Typed values
    # - type
    @overload
    def __new__(
        cls, key: None, value: Type[VT], default: Optional[TDict[Any, Any]] = None
    ) -> DefaultDict[Any, VT]: ...
    # - 1-tuple
    @overload
    def __new__(
        cls,
        key: None,
        value: Tuple[Type[VT]],
        default: Optional[TDict[Any, Any]] = None,
    ) -> DefaultDict[Any, VT]: ...
    # - 2-tuple
    @overload
    def __new__(
        cls,
        key: None,
        value: Tuple[Type[VT], Type[VT1]],
        default: Optional[TDict[Any, Any]] = None,
    ) -> DefaultDict[Any, VT | VT1]: ...
    # - 3-tuple
    @overload
    def __new__(
        cls,
        key: None,
        value: Tuple[Type[VT], Type[VT1], Type[VT2]],
        default: Optional[TDict[Any, Any]] = None,
    ) -> DefaultDict[Any, VT | VT1 | VT2]: ...
    # - member
    @overload
    def __new__(
        cls,
        key: None,
        value: Member[VT, Any],
        default: Optional[TDict[Any, Any]] = None,
    ) -> DefaultDict[Any, VT]: ...
    # Typed value through keyword
    # - type
    @overload
    def __new__(
        cls,
        key: None = None,
        *,
        value: Type[VT],
        default: Optional[TDict[Any, Any]] = None,
    ) -> DefaultDict[Any, VT]: ...
    # - 1-tuple
    @overload
    def __new__(
        cls,
        key: None = None,
        *,
        value: Tuple[Type[VT]],
        default: Optional[TDict[Any, Any]] = None,
    ) -> DefaultDict[Any, VT]: ...
    # - 2-tuple
    @overload
    def __new__(
        cls,
        key: None = None,
        *,
        value: Tuple[Type[VT], Type[VT1]],
        default: Optional[TDict[Any, Any]] = None,
    ) -> DefaultDict[Any, VT | VT1]: ...
    # - 3-tuple
    @overload
    def __new__(
        cls,
        key: None = None,
        *,
        value: Tuple[Type[VT], Type[VT1], Type[VT2]],
        default: Optional[TDict[Any, Any]] = None,
    ) -> DefaultDict[Any, VT | VT1 | VT2]: ...
    # - member
    @overload
    def __new__(
        cls,
        key: None = None,
        *,
        value: Member[VT, Any],
        default: Optional[TDict[Any, Any]] = None,
    ) -> DefaultDict[Any, VT]: ...
    # Typed key and value
    # - value simple type
    #    - key type
    @overload
    def __new__(
        cls, key: Type[KT], value: Type[VT], default: Optional[TDict[Any, Any]] = None
    ) -> DefaultDict[KT, VT]: ...
    #    - key 1-tuple
    @overload
    def __new__(
        cls,
        key: Tuple[Type[KT]],
        value: Type[VT],
        default: Optional[TDict[Any, Any]] = None,
    ) -> DefaultDict[KT, VT]: ...
    #    - key 2-tuple
    @overload
    def __new__(
        cls,
        key: Tuple[Type[KT], Type[KT1]],
        value: Type[VT],
        default: Optional[TDict[Any, Any]] = None,
    ) -> DefaultDict[KT | KT1, VT]: ...
    #    - key 3-tuple
    @overload
    def __new__(
        cls,
        key: Tuple[Type[KT], Type[KT1], Type[KT2]],
        value: Type[VT],
        default: Optional[TDict[Any, Any]] = None,
    ) -> DefaultDict[KT | KT1 | KT2, VT]: ...
    #    - key member
    @overload
    def __new__(
        cls,
        key: Member[KT, Any],
        value: Type[VT],
        default: Optional[TDict[Any, Any]] = None,
    ) -> DefaultDict[KT, VT]: ...
    # - Value as single element tuple
    #    - key type
    @overload
    def __new__(
        cls,
        key: Type[KT],
        value: Tuple[Type[VT]],
        default: Optional[TDict[Any, Any]] = None,
    ) -> DefaultDict[KT, VT]: ...
    #    - key 1-tuple
    @overload
    def __new__(
        cls,
        key: Tuple[Type[KT]],
        value: Tuple[Type[VT]],
        default: Optional[TDict[Any, Any]] = None,
    ) -> DefaultDict[KT, VT]: ...
    #    - key 2-tuple
    @overload
    def __new__(
        cls,
        key: Tuple[Type[KT], Type[KT1]],
        value: Tuple[Type[VT]],
        default: Optional[TDict[Any, Any]] = None,
    ) -> DefaultDict[KT | KT1, VT]: ...
    #    - key 3-tuple
    @overload
    def __new__(
        cls,
        key: Tuple[Type[KT], Type[KT1], Type[KT2]],
        value: Tuple[Type[VT]],
        default: Optional[TDict[Any, Any]] = None,
    ) -> DefaultDict[KT | KT1 | KT2, VT]: ...
    #    - key member
    @overload
    def __new__(
        cls,
        key: Member[KT, Any],
        value: Tuple[Type[VT]],
        default: Optional[TDict[Any, Any]] = None,
    ) -> DefaultDict[KT, VT]: ...
    # - Value as 2-tuple
    #    - key type
    @overload
    def __new__(
        cls,
        key: Type[KT],
        value: Tuple[Type[VT], Type[VT1]],
        default: Optional[TDict[Any, Any]] = None,
    ) -> DefaultDict[KT, VT | VT1]: ...
    #    - key 1-tuple
    @overload
    def __new__(
        cls,
        key: Tuple[Type[KT]],
        value: Tuple[Type[VT], Type[VT1]],
        default: Optional[TDict[Any, Any]] = None,
    ) -> DefaultDict[KT, VT | VT1]: ...
    #    - key 2-tuple
    @overload
    def __new__(
        cls,
        key: Tuple[Type[KT], Type[KT1]],
        value: Tuple[Type[VT], Type[VT1]],
        default: Optional[TDict[Any, Any]] = None,
    ) -> DefaultDict[KT | KT1, VT | VT1]: ...
    #    - key 3-tuple
    @overload
    def __new__(
        cls,
        key: Tuple[Type[KT], Type[KT1], Type[KT2]],
        value: Tuple[Type[VT], Type[VT1]],
        default: Optional[TDict[Any, Any]] = None,
    ) -> DefaultDict[KT | KT1 | KT2, VT | VT1]: ...
    #    - key member
    @overload
    def __new__(
        cls,
        key: Member[KT, Any],
        value: Tuple[Type[VT], Type[VT1]],
        default: Optional[TDict[Any, Any]] = None,
    ) -> DefaultDict[KT, VT | VT1]: ...
    # - Value as 3-tuple
    #   - key type
    @overload
    def __new__(
        cls,
        key: Type[KT],
        value: Tuple[Type[VT], Type[VT1], Type[VT2]],
        default: Optional[TDict[Any, Any]] = None,
    ) -> DefaultDict[KT, VT | VT1 | VT2]: ...
    #   - key 1-tuple
    @overload
    def __new__(
        cls,
        key: Tuple[Type[KT]],
        value: Tuple[Type[VT], Type[VT1], Type[VT2]],
        default: Optional[TDict[Any, Any]] = None,
    ) -> DefaultDict[KT, VT | VT1 | VT2]: ...
    #   - key 2-tuple
    @overload
    def __new__(
        cls,
        key: Tuple[Type[KT], Type[KT1]],
        value: Tuple[Type[VT], Type[VT1], Type[VT2]],
        default: Optional[TDict[Any, Any]] = None,
    ) -> DefaultDict[KT | KT1, VT | VT1 | VT2]: ...
    #   - key 3-tuple
    @overload
    def __new__(
        cls,
        key: Tuple[Type[KT], Type[KT1], Type[KT2]],
        value: Tuple[Type[VT], Type[VT1], Type[VT2]],
        default: Optional[TDict[Any, Any]] = None,
    ) -> DefaultDict[KT | KT1 | KT2, VT | VT1 | VT2]: ...
    #   - key member
    @overload
    def __new__(
        cls,
        key: Member[KT, Any],
        value: Tuple[Type[VT], Type[VT1], Type[VT2]],
        default: Optional[TDict[Any, Any]] = None,
    ) -> DefaultDict[KT, VT | VT1 | VT2]: ...
    # - value as member
    #   - key type
    @overload
    def __new__(
        cls,
        key: Type[KT],
        value: Member[VT, Any],
        default: Optional[TDict[Any, Any]] = None,
    ) -> DefaultDict[KT, VT]: ...
    #   - key 1-tuple
    @overload
    def __new__(
        cls,
        key: Tuple[Type[KT]],
        value: Member[VT, Any],
        default: Optional[TDict[Any, Any]] = None,
    ) -> DefaultDict[KT, VT]: ...
    #   - key 2-tuple
    @overload
    def __new__(
        cls,
        key: Tuple[Type[KT], Type[KT1]],
        value: Member[VT, Any],
        default: Optional[TDict[Any, Any]] = None,
    ) -> DefaultDict[KT | KT1, VT]: ...
    #   - key 3-tuple
    @overload
    def __new__(
        cls,
        key: Tuple[Type[KT], Type[KT1], Type[KT2]],
        value: Member[VT, VT],
        default: Optional[TDict[Any, Any]] = None,
    ) -> DefaultDict[KT | KT1 | KT2, VT]: ...
    #   - key member
    @overload
    def __new__(
        cls,
        key: Member[KT, KT],
        value: Member[VT, VT],
        default: Optional[TDict[Any, Any]] = None,
    ) -> DefaultDict[KT, VT]: ...
