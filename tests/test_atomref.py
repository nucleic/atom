# --------------------------------------------------------------------------------------
# Copyright (c) 2018-2024, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
import gc

import pytest

from atom.api import Atom, atomref


def test_live_atomref():
    """Test a live atomref."""
    atom = Atom()
    ref = atomref(atom)

    assert ref is atomref(atom)
    assert ref and ref() is atom
    assert "AtomRef" in repr(ref)

    ref.__sizeof__()

    with pytest.raises(TypeError):
        atomref(object())


def test_dead_atomref():
    """Test a dead atomref."""
    atom = Atom()
    ref = atomref(atom)
    del atom
    gc.collect()

    assert not ref and ref() is None
    assert "AtomRef" in repr(ref)

    ref.__sizeof__()


def test_atomref_errors():
    """Test invalid arguments"""
    a = Atom()

    with pytest.raises(TypeError) as excinfo:
        atomref()
    assert "argument" in excinfo.exconly()
    with pytest.raises(TypeError) as excinfo:
        atomref(None)
    assert "CAtom" in excinfo.exconly()

    ref = atomref(a)
    with pytest.raises(TypeError):
        ref(atom=a)
    with pytest.raises(TypeError):
        ref(a)
