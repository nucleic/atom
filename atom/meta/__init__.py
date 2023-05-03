# --------------------------------------------------------------------------------------
# Copyright (c) 2023, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
"""Atom metaclass and tools used to create atom subclasses."""
from .atom_meta import AtomMeta, MissingMemberWarning, add_member, clone_if_needed
from .member_modifiers import set_default
from .observation import observe

__all__ = [
    "AtomMeta",
    "observe",
    "MissingMemberWarning",
    "add_member",
    "set_default",
    "clone_if_needed",
]
