#------------------------------------------------------------------------------
# Copyright (c) 2013-2017, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
"""Test the set behaviors

    no_op_handler: not sure if it is used
    slot_handler: standard one tested through other tests (post_set, ...)
    constant_handler
    read_only_handler
    event_handler: not tested here (see test_observe.py)
    signal_handler
    delegate_handler: not tested here (see test_delegate.py)
    property_handler: not tested here (see test_property.py)
    call_object_object_value_handler: not used as far as I can tell
    call_object_object_name_value_handler: not used as far as I can tell
    object_method_value_handler: not used as far as I can tell
    object_method_name_value_handler: not used as far as I can tell
    member_method_object_value_handler: method defined on a Member subclass

"""
import pytest
from atom.api import (Atom, Int, Constant, Signal, ReadOnly, Event, SetAttr)


@pytest.mark.parametrize("member", [(Signal(),), (Constant(1),)])
def test_unsettable(member):
    """Test that unsettable members do raise the proper error.

    """
    class Unsettable(Atom):

        m = member

    u = Unsettable()
    with pytest.raises(AttributeError):
        u.m = None


def test_read_only_behavior():
    """Test the behavior of read only member.

    """
    class ReadOnlyTest(Atom):

        r = ReadOnly()

    rt = ReadOnlyTest()
    rt.r = 1
    assert rt.r == 1
    with pytest.raises(TypeError):
        rt.r = 2


def test_member_set_behaviors():
    """Test defining set in a Member subclass

    """
    class ModuloInt(Int):

        def __init__(self):
            super(ModuloInt, self).__init__()
            mode = SetAttr.MemberMethod_ObjectValue
            self.set_setattr_mode(mode, 'set')

        def set(self, obj, value):
            self.set_slot(obj, value % 2)

    class SetTest(Atom):

        mi = ModuloInt()

    pvt = SetTest()
    mi = pvt.get_member('mi')
    assert mi.setattr_mode[0] == SetAttr.MemberMethod_ObjectValue
    pvt.mi = 2
    assert pvt.mi == 0
    pvt.mi = 3
    assert pvt.mi == 1
