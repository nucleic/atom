#------------------------------------------------------------------------------
# Copyright (c) 2013-2017, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
"""Test the delegator member.

We need to test all behaviors:
    - get
    - post_get
    - set
    - post_set
    - post_validate
    - default_value
    - del

"""
import pytest
from atom.api import (Atom, Int, Delegator, GetAttr, PostGetAttr, SetAttr,
                      PostSetAttr, PostValidate, DefaultValue)
from atom.catom import DelAttr


class TrackedInt(Int):
    """Member used to check that a Delegate does forward all calls.

    """

    def __init__(self):
        super(TrackedInt, self).__init__()
        self.called = []
        mode = GetAttr.MemberMethod_Object
        self.set_getattr_mode(mode, 'get')
        mode = PostGetAttr.MemberMethod_ObjectValue
        self.set_post_getattr_mode(mode, 'post_get')
        mode = SetAttr.MemberMethod_ObjectValue
        self.set_setattr_mode(mode, 'set')
        mode = PostSetAttr.MemberMethod_ObjectOldNew
        self.set_post_setattr_mode(mode, 'post_set')
        mode = PostValidate.MemberMethod_ObjectOldNew
        self.set_post_validate_mode(mode, 'post_validate')
        mode = DefaultValue.MemberMethod_Object
        self.set_default_value_mode(mode, 'default_value')

    def get(self, obj):
        self.called.append('get')
        return self.get_slot(obj)

    def post_get(self, obj, value):
        self.called.append('post_get')
        return value

    def set(self, obj, value):
        self.called.append('set')
        self.set_slot(obj)

    def post_set(self, obj, old, new):
        self.called.append('post_set')

    def post_validate(self, obj, old, new):
        self.called.append('post_validate')

    def default_value(self, obj):
        self.called.append('default_value')
        return 0


def test_delegator_behaviors():
    """
    """
    class DelegateTest(Atom):

        tracked_int = TrackedInt()

        d = Delegator(tracked_int)

    # Test also delegating get and set


def test_delegator_methods():
    """
    """
    pass


def test_delegator_cloning():
    """
    """
    pass
