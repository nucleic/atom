# --------------------------------------------------------------------------------------
# Copyright (c) 2023, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
"""Custom marker objects used to modify the default settings of a member."""
from typing import Any, Optional


class set_default(object):
    """An object used to set the default value of a base class member."""

    __slots__ = ("value", "name")

    #: Name of the member for which a new default value should be set. Used by
    #: the metaclass.
    name: Optional[str]

    #: New default value to be set.
    value: Any

    def __init__(self, value: Any) -> None:
        self.value = value
        self.name = None  # storage for the metaclass

    def clone(self) -> "set_default":
        """Create a clone of the sentinel."""
        return type(self)(self.value)
