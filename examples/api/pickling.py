# --------------------------------------------------------------------------------------
# Copyright (c) 2023, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
"""Demonstration of the basic use of the Coerced member.

"""
import pickle

from atom.api import Atom, GetState, clone_if_needed


class PicklePublicOnly(Atom):
    def __init_subclass__(cls) -> None:
        super().__init_subclass__()
        for k, m in cls.members().items():
            if k.startswith("_"):
                m = clone_if_needed(cls, m)
                m.set_getstate_mode(GetState.Exclude, None)


class Demo(PicklePublicOnly):
    #: Public member declared using type annotation
    public: int

    #: Private member declared using type annotation
    _private: int


d = Demo(public=10, _private=20)
print(d.public)  # 10
print(d._private)  # 20

new_d = pickle.loads(pickle.dumps(d))
print(new_d.public)  # 10
print(new_d._private)  # 0: was not pickled
