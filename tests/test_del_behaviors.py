#------------------------------------------------------------------------------
# Copyright (c) 2013-2017, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
"""Test the del behaviors

    no_op_handler:  not sure it is used
    slot_handler: behavior leading to calling the default factory on next get
    constant_handler
    read_only_handler
    event_handler
    signal_handler
    delegate_handler: not tested here (see test_delegate.py)
    property_handler: not tested here (see test_property.py)

"""
import pytest
from atom.api import (Atom, Int, Constant, Signal, ReadOnly, Event)
from atom.catom import DelAttr


@pytest.mark.parametrize("member, mode",
                         [(Event(), DelAttr.Event), (Signal(), DelAttr.Signal),
                          (ReadOnly(), DelAttr.ReadOnly),
                          (Constant(1), DelAttr.Constant)
                          ])
def test_undeletable(member, mode):
    """Test that unsettable members do raise the proper error.

    """
    class Undeletable(Atom):

        m = member

    assert Undeletable.m.delattr_mode[0] == mode
    u = Undeletable()
    with pytest.raises(TypeError):
        del u.m
    with pytest.raises(TypeError):
        Undeletable.m.do_delattr(u)


def test_del_slot():
    """Test deleting a member using the slot handler.

    """
    class DelSlot(Atom):

        i = Int(10)

    a = DelSlot()
    assert a.i == 10
    a.i = 0
    del a.i
    assert a.i == 10
