#------------------------------------------------------------------------------
# Copyright (c) 2013-2017, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
"""Test post_validate/get/set behaviors

All of them have the following handlers:
    no_op_handler
    delegate_handler: not tested here (see test_delegate.py)
    object_method_old_new_handler: method defined on the Atom object
    object_method_name_old_new_handler: not used as far as I can tell
    member_method_object_old_new_handler: Method defined on a Member subclass

"""
from atom.api import (Atom, Int, PostValidate, PostGetAttr, PostSetAttr)


def test_post_validate():
    """Test the post_validate behaviors.

    """
    class ModuloInt(Int):

        def __init__(self):
            super(ModuloInt, self).__init__()
            mode = PostValidate.MemberMethod_ObjectOldNew
            self.set_post_validate_mode(mode, 'post_validate')

        def post_validate(self, obj, old, new):
            return new % 2

    class PostValidateTest(Atom):

        mi = ModuloInt()

        manual_mi = Int()

        def _post_validate_manual_mi(self, old, new):
            return new % 2

    # Test subclassed member
    pvt = PostValidateTest()
    mi = pvt.get_member('mi')
    assert mi.post_validate_mode[0] == PostValidate.MemberMethod_ObjectOldNew
    pvt.mi = 2
    assert pvt.mi == 0
    pvt.mi = 3
    assert pvt.mi == 1

    # Test do_post_validate
    assert mi.do_post_validate(pvt, pvt.mi, 2) == 0
    assert mi.do_post_validate(pvt, pvt.mi, 3) == 1

    # Test class defined custom post_validator
    mmi = pvt.get_member('manual_mi')
    assert mmi.post_validate_mode[0] == PostValidate.ObjectMethod_OldNew
    pvt.manual_mi = 2
    assert pvt.manual_mi == 0
    pvt.manual_mi = 3
    assert pvt.manual_mi == 1

    # Test do_full_validate
    assert mmi.do_post_validate(pvt, pvt.manual_mi, 2) == 0
    assert mmi.do_post_validate(pvt, pvt.manual_mi, 3) == 1


def test_post_getattr():
    """Test the post_getattr behaviors.

    """
    class ModuloInt(Int):

        def __init__(self):
            super(ModuloInt, self).__init__()
            mode = PostGetAttr.MemberMethod_ObjectValue
            self.set_post_getattr_mode(mode, 'post_getattr')

        def post_getattr(self, obj, value):
            return value % 2

    class PostGetattrTest(Atom):

        mi = ModuloInt()

        manual_mi = Int()

        def _post_getattr_manual_mi(self, value):
            return value % 2

    # Test subclassed member
    pvt = PostGetattrTest()
    mi = pvt.get_member('mi')
    assert mi.post_getattr_mode[0] == PostGetAttr.MemberMethod_ObjectValue
    pvt.mi = 2
    assert pvt.mi == 0
    pvt.mi = 3
    assert pvt.mi == 1

    # Test class defined custom post_getattr
    mmi = pvt.get_member('manual_mi')
    assert mmi.post_getattr_mode[0] == PostGetAttr.ObjectMethod_Value
    pvt.manual_mi = 2
    assert pvt.manual_mi == 0
    pvt.manual_mi = 3
    assert pvt.manual_mi == 1

    # Test do_post_getattr
    assert mi.do_post_getattr(pvt, 2) == 0
    assert mi.do_post_getattr(pvt, 3) == 1


def test_post_setattr():
    """Test the post_setattr behaviors.

    """
    class TrackedInt(Int):

        def __init__(self):
            super(TrackedInt, self).__init__()
            mode = PostSetAttr.MemberMethod_ObjectOldNew
            self.set_post_setattr_mode(mode, 'post_setattr')

        def post_setattr(self, obj, old, new):
            obj.counter += 1

    class PostSetattrTest(Atom):

        mi = TrackedInt()

        manual_mi = Int()

        counter = Int()

        def _post_setattr_manual_mi(self, old, new):
            self.counter += 1

    # Test subclassed member
    pvt = PostSetattrTest()
    mi = pvt.get_member('mi')
    assert mi.post_setattr_mode[0] == PostSetAttr.MemberMethod_ObjectOldNew
    pvt.mi = 2
    assert pvt.counter == 1
    pvt.mi = 3
    assert pvt.counter == 2

    # Test class defined custom post_setattr
    mmi = pvt.get_member('manual_mi')
    assert mmi.post_setattr_mode[0] == PostSetAttr.ObjectMethod_OldNew
    pvt.manual_mi = 2
    assert pvt.counter == 3
    pvt.manual_mi = 3
    assert pvt.counter == 4

    # Test do_post_setattr
    mmi.do_post_setattr(pvt, pvt.manual_mi, 2)
    assert pvt.counter == 5
    mmi.do_post_setattr(pvt, pvt.manual_mi, 3)
    assert pvt.counter == 6
