# --------------------------------------------------------------------------------------
# Copyright (c) 2018-2023, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
"""Test the signal connectors.

"""
import gc
import operator
import sys
from collections import Counter

import pytest

from atom.api import Atom, Signal


def test_signalconnector_lifecycle():
    """Test creating and destroying an event binder.

    We create enough event binder to exceed the freelist length and fully
    deallocate some.

    """

    class SignalAtom(Atom):
        s = Signal()

    signal_connectors = [SignalAtom.s for i in range(512)]
    for i, e in enumerate(signal_connectors):
        signal_connectors[i] = None
        del e
        gc.collect()

    atom = SignalAtom()
    sc = atom.s
    # Under Python 3.9+ heap allocated type instance keep a reference to the
    # type
    referents = [SignalAtom.s, atom]
    if sys.version_info >= (3, 9):
        referents.append(type(sc))
    assert Counter(gc.get_referents(sc)) == Counter(referents)


def test_signalconnector_cmp():
    """Test comparing event binders."""

    class EventAtom(Atom):
        s1 = Signal()
        s2 = Signal()

    a = EventAtom()
    assert a.s1 == a.s1
    assert not a.s1 == a.s2
    assert not a.s1 == 1

    if sys.version_info >= (3,):
        for op in ("lt", "le", "gt", "ge"):
            with pytest.raises(TypeError):
                getattr(operator, op)(a.s1, 1)
