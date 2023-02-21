# --------------------------------------------------------------------------------------
# Copyright (c) 2018-2023, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
"""Test the notification mechanisms.

"""
import gc
import operator
import sys
from collections import Counter

import pytest

from atom.api import Atom, Event, Int


def test_eventbinder_lifecycle():
    """Test creating and destroying an event binder.

    We create enough event binder to exceed the freelist length and fully
    deallocate some.

    """

    class EventAtom(Atom):
        e = Event()

    event_binders = [EventAtom.e for i in range(512)]
    for i, e in enumerate(event_binders):
        event_binders[i] = None
        del e
        gc.collect()

    atom = EventAtom()
    eb = atom.e
    # Under Python 3.9+ heap allocated type instance keep a reference to the
    # type
    referents = [EventAtom.e, atom]
    if sys.version_info >= (3, 9):
        referents.append(type(eb))
    assert Counter(gc.get_referents(eb)) == Counter(referents)


def test_eventbinder_call():
    """Test calling an event binder and handling bad arguments."""

    class EventAtom(Atom):
        counter = Int()
        e = Event()

    a = EventAtom()

    def update_counter(change):
        change["object"].counter += change["value"]

    a.observe("e", update_counter)

    a.e(2)
    assert a.counter == 2

    with pytest.raises(TypeError) as excinfo:
        a.e(k=1)
    assert "keyword arguments" in excinfo.exconly()

    with pytest.raises(TypeError) as excinfo:
        a.e(1, 2, 3)
    assert "at most 1 argument" in excinfo.exconly()


def test_eventbinder_cmp():
    """Test comparing event binders."""

    class EventAtom(Atom):
        e1 = Event()
        e2 = Event()

    a = EventAtom()
    assert a.e1 == a.e1
    assert not a.e1 == a.e2
    assert not a.e1 == 1

    if sys.version_info >= (3,):
        for op in ("lt", "le", "gt", "ge"):
            with pytest.raises(TypeError):
                getattr(operator, op)(a.e1, 1)
