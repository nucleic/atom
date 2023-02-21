# --------------------------------------------------------------------------------------
# Copyright (c) 2013-2023, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
"""Test post_validate/get/set behaviors

All of them have the following handlers:
    no_op_handler: Do nothing
    delegate_handler: not tested here (see test_delegate.py)
    object_method_old_new_handler: method defined on the Atom object
    object_method_name_old_new_handler: method defined on the Atom object
                                        taking the member name as first arg
    member_method_object_old_new_handler: Method defined on a Member subclass

"""
import pytest

from atom.api import Int, PostGetAttr, PostSetAttr, PostValidate

GET_MEMBER_METHOD_SRC = """
from atom.api import Atom

class TrackedInt(Int):

    def __init__(self):
        super(TrackedInt, self).__init__()
        mode = PostGetAttr.{mode}
        self.set_post_getattr_mode(mode, 'post_getattr')

    def post_getattr(self, obj, value):
        obj.counter += 1

class PostAtom(Atom):

    mi = TrackedInt()

    counter = Int()

"""

GET_OBJECT_METHOD_SRC = """
from atom.api import Atom

class PostAtom(Atom):

    mi = Int()

    counter = Int()

    def _post_getattr_mi(self, value):
        self.counter += 1
"""

GET_OBJECT_METHOD_NAME_SRC = """
from atom.api import Atom

class PostAtom(Atom):

    mi = Int()
    mi.set_post_getattr_mode(getattr(PostGetAttr, 'ObjectMethod_NameValue'),
                                     'post_getattr_mi')

    counter = Int()

    def post_getattr_mi(self, name, value):
        self.counter += 1
"""


@pytest.mark.parametrize(
    "mode",
    (
        "NoOp",
        "ObjectMethod_Value",
        "ObjectMethod_NameValue",
        "MemberMethod_ObjectValue",
    ),
)
def test_post_getattr(mode):
    """Test the post_getattr_behaviors."""
    namespace = {}
    namespace.update(globals())
    src = (
        GET_MEMBER_METHOD_SRC
        if mode in ("MemberMethod_ObjectValue", "NoOp")
        else GET_OBJECT_METHOD_SRC
        if mode == "ObjectMethod_Value"
        else GET_OBJECT_METHOD_NAME_SRC
    )
    src = src.format(mode=mode)
    print(src)
    exec(src, namespace)

    PostAtom = namespace["PostAtom"]

    # Test subclassed member
    pot = PostAtom()
    mi = pot.get_member("mi")
    assert mi.post_getattr_mode[0] == getattr(PostGetAttr, mode)
    pot.mi
    assert pot.counter == (1 if mode != "NoOp" else 0)

    # Test do_post_*** method
    func = getattr(mi, "do_post_getattr")
    func(pot, 2)
    assert pot.counter == (2 if mode != "NoOp" else 0)


MEMBER_METHOD_SRC = """
from atom.api import Atom

class TrackedInt(Int):

    def __init__(self):
        super(TrackedInt, self).__init__()
        mode = enum.{mode}
        self.set_post_{operation}_mode(mode, 'post_{operation}')

    def post_{operation}(self, obj, old, new):
        obj.counter += 1

class PostAtom(Atom):

    mi = TrackedInt()

    counter = Int()

"""

OBJECT_METHOD_SRC = """
from atom.api import Atom

class PostAtom(Atom):

    mi = Int()

    counter = Int()

    def _post_{operation}_mi(self, old, new):
        self.counter += 1
"""

OBJECT_METHOD_NAME_SRC = """
from atom.api import Atom

class PostAtom(Atom):

    mi = Int()
    mi.set_post_{operation}_mode(getattr(enum, 'ObjectMethod_NameOldNew'),
                                        'post_{operation}_mi')

    counter = Int()

    def post_{operation}_mi(self, name, old, new):
        self.counter += 1
"""


@pytest.mark.parametrize(
    "operation, enum", [("setattr", PostSetAttr), ("validate", PostValidate)]
)
@pytest.mark.parametrize(
    "mode",
    (
        "NoOp",
        "ObjectMethod_OldNew",
        "ObjectMethod_NameOldNew",
        "MemberMethod_ObjectOldNew",
    ),
)
def test_post_setattr_validate(operation, enum, mode):
    """Test the post_setattr/validate behaviors."""
    namespace = {"enum": enum}
    namespace.update(globals())
    src = (
        MEMBER_METHOD_SRC
        if mode in ("MemberMethod_ObjectOldNew", "NoOp")
        else OBJECT_METHOD_SRC
        if mode == "ObjectMethod_OldNew"
        else OBJECT_METHOD_NAME_SRC
    )
    src = src.format(operation=operation, mode=mode)
    print(src)
    exec(src, namespace)

    PostAtom = namespace["PostAtom"]

    # Test subclassed member
    pot = PostAtom()
    mi = pot.get_member("mi")
    assert getattr(mi, "post_{}_mode".format(operation))[0] == getattr(enum, mode)
    pot.mi = 2
    assert pot.counter == (1 if mode != "NoOp" else 0)

    # Test do_post_*** method
    func = getattr(mi, "do_post_{}".format(operation))
    func(pot, 2, 3)
    assert pot.counter == (2 if mode != "NoOp" else 0)


@pytest.mark.parametrize(
    "operation, mode, msg",
    [
        ("getattr", PostGetAttr.ObjectMethod_Value, "str"),
        ("getattr", PostGetAttr.ObjectMethod_NameValue, "str"),
        ("getattr", PostGetAttr.MemberMethod_ObjectValue, "str"),
        ("setattr", PostSetAttr.ObjectMethod_OldNew, "str"),
        ("setattr", PostSetAttr.ObjectMethod_NameOldNew, "str"),
        ("setattr", PostSetAttr.MemberMethod_ObjectOldNew, "str"),
        ("validate", PostValidate.ObjectMethod_OldNew, "str"),
        ("validate", PostValidate.ObjectMethod_NameOldNew, "str"),
        ("validate", PostValidate.MemberMethod_ObjectOldNew, "str"),
    ],
)
def test_wrong_argument_in_mode_setting(operation, mode, msg):
    """Test handling wrong argument types when setting mode."""
    m = Int()
    with pytest.raises(TypeError) as excinfo:
        getattr(m, "set_post_{}_mode".format(operation))(mode, 1)
    assert msg in excinfo.exconly()
