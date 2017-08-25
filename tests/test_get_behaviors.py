#------------------------------------------------------------------------------
# Copyright (c) 2013-2017, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
"""Test the get behaviors

    no_op_handler: : not sure if it is used
    slot_handler: standard one tested through other tests (post_get, ...)
    event_handler: not tested here (see test_observe.py)
    signal_handler: not tested here (see test_observe.py)
    delegate_handler: not tested here (see test_delegate.py)
    property_handler: not tested here (see test_property.py)
    cached_property_handler: not tested here (see test_property.py)
    call_object_object_handler: not used as far as I can tell
    call_object_object_name_handler: not used as far as I can tell
    object_method_handler: not used as far as I can tell
    object_method_name_handler: not used as far as I can tell
    member_method_object_handler: method defined on a Member subclass

"""
from atom.api import (Atom, Int, GetAttr)


def test_get_behaviors():
    """Test defining get in a Member subclass

    """
    class ModuloInt(Int):

        def __init__(self):
            super(ModuloInt, self).__init__()
            mode = GetAttr.MemberMethod_Object
            self.set_getattr_mode(mode, 'get')

        def get(self, obj):
            return self.get_slot(obj) % 2

    class GetTest(Atom):

        mi = ModuloInt()

    pvt = GetTest()
    mi = pvt.get_member('mi')
    assert mi.getattr_mode[0] == GetAttr.MemberMethod_Object
    pvt.mi = 2
    assert pvt.mi == 0
    pvt.mi = 3
    assert pvt.mi == 1

    pvt.mi = 2
    assert mi.do_getattr(pvt) == 0
    pvt.mi = 3
    assert mi.do_getattr(pvt) == 1
