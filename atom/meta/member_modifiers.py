# --------------------------------------------------------------------------------------
# Copyright (c) 2023-2024, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
"""Custom marker objects used to modify the default settings of a member."""

from typing import Any, Callable, Optional, Self

_SENTINEL = object()


class member(object):
    """An object used to configure a member defined by a type annotation."""

    __slots__ = (
        "coercer",
        "default_args",
        "default_factory",
        "default_kwargs",
        "default_value",
        "metadata",
        "name",
    )

    #: Name of the member for which a new default value should be set. Used by
    #: the metaclass.
    name: Optional[str]

    #: Default value.
    default_value: Any

    #: Default value factory.
    default_factory: Optional[Callable[[], Any]]

    #: Tuple of argument to create a default value.
    default_args: Optional[tuple]

    #: Keyword arguments to create a default value.
    default_kwargs: Optional[dict]

    #: Coercing function to use.
    coercer: Optional[Callable[[Any], Any]]

    #: Metadata to set on the member
    metadata: dict[str, Any]

    def __init__(
        self,
        default_value: Any = _SENTINEL,
        default_factory: Optional[Callable[[], Any]] = None,
        default_args: Optional[tuple] = None,
        default_kwargs: Optional[dict] = None,
        coercer: Optional[Callable[[Any], Any]] = None,
    ) -> None:
        if default_value is not _SENTINEL:
            if (
                default_factory is not None
                or default_args is not None
                or default_kwargs is not None
            ):
                raise ValueError(
                    "Cannot specify a default value and a factory or args or kwargs"
                )
        elif default_factory is not None:
            if default_args is not None or default_kwargs is not None:
                raise ValueError("Cannot specify a factory and args or kwargs")
        self.name = None  # storage for the metaclass
        self.default_value = default_value
        self.default_factory = default_factory
        self.default_args = default_args
        self.default_kwargs = default_kwargs
        self.coercer = coercer
        self.metadata = {}

    def clone(self) -> Self:
        """Create a clone of the sentinel."""
        new = type(self)(
            self.default_value,
            self.default_factory,
            self.default_args,
            self.default_kwargs,
            self.coercer,
        )
        new.metadata = self.metadata.copy()
        return new

    def tag(self, **meta: Any) -> Self:
        """Add new metadata to the member."""
        self.metadata |= meta
        return self


def set_default(value: Any) -> member:
    return member(default_value=value)
