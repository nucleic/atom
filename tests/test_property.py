# --------------------------------------------------------------------------------------
# Copyright (c) 2013-2023, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
"""Test the property and cached property member

"""
import pytest

from atom.api import (
    Atom,
    GetAttr,
    Int,
    Property,
    SetAttr,
    Value,
    cached_property,
    observe,
)
from atom.catom import DelAttr, reset_property


def test_property1():
    """Test defining a property using the arguments."""

    def get_function(obj):
        return obj.i

    def set_function(obj, value):
        obj.i = value

    def del_function(obj):
        del obj.i

    class PropertyTest(Atom):
        p = Property(get_function, set_function, del_function)

        i = Int()

    assert not PropertyTest.p.cached
    assert PropertyTest.p.fget is get_function
    assert PropertyTest.p.fset is set_function
    assert PropertyTest.p.fdel is del_function
    pt = PropertyTest()
    assert pt.p == 0
    pt.p = 10
    assert pt.i == 10
    assert pt.p == 10
    del pt.p
    assert pt.p == 0


def test_property2():
    """Test defining a property using the decorators."""

    class PropertyTest(Atom):
        p = Property()

        i = Int()

        @p.getter
        def get_function(obj):
            return obj.i

        @p.setter
        def set_function(obj, value):
            obj.i = value

        @p.deleter
        def del_function(obj):
            del obj.i

    pt = PropertyTest()
    assert pt.p == 0
    pt.p = 10
    assert pt.i == 10
    assert pt.p == 10
    del pt.p
    assert pt.p == 0


def test_property3():
    """Test defining a property mangled method names."""

    class PropertyTest(Atom):
        p = Property()

        i = Int()

        def _get_p(self):
            return self.i

        def _set_p(self, value):
            self.i = value

        def _del_p(self):
            del self.i

    pt = PropertyTest()
    assert pt.p == 0
    pt.p = 10
    assert pt.i == 10
    assert pt.p == 10
    del pt.p
    assert pt.p == 0


def test_property4():
    """Test handling missing function(fget, fset, fdel)"""

    class PropertyTest(Atom):
        p = Property()

    pt = PropertyTest()
    with pytest.raises(AttributeError):
        pt.p

    with pytest.raises(AttributeError):
        pt.p = 1

    with pytest.raises(AttributeError):
        del pt.p


def test_cached_property():
    """Test using a cached property."""

    class PropertyTest(Atom):
        i = Int()

        @cached_property
        def prop(self):
            self.i += 1
            return self.i

    assert PropertyTest.prop.cached
    pt = PropertyTest()
    assert pt.prop == 1
    assert pt.prop == 1
    pt.get_member("prop").reset(pt)
    assert pt.prop == 2


def test_enforce_read_only_cached_property():
    """Check a cached property has to be read-only."""

    def get(self):
        pass

    def set(self, value):
        pass

    with pytest.raises(ValueError):
        Property(get, set, cached=True)

    with pytest.raises(ValueError):
        p = Property(cached=True)
        p.setter(set)


def test_observed_property():
    """Test observing a property."""

    class NonComparableObject:
        def __eq__(self, other):
            raise ValueError()

        def __add__(self, other):
            return other + 5

    class PropertyTest(Atom):
        i = Value(0)

        counter = Int()

        prop = Property()

        @prop.getter
        def _prop(self):
            self.i += 1
            return self.i

        @observe("prop")
        def observe_cp(self, change):
            self.counter += 1

    pt = PropertyTest()
    assert pt.prop == 1
    assert pt.prop == 2

    pt.i = NonComparableObject()
    pt.observe("prop", pt.observe_cp)
    pt.get_member("prop").reset(pt)
    assert pt.counter == 2
    assert pt.prop == 7


def test_wrong_reset_arguments():
    """Test the handling of wrong arguments in reset."""
    prop = Property()

    with pytest.raises(TypeError) as excinfo:
        reset_property()
    assert "2 arguments" in excinfo.exconly()

    with pytest.raises(TypeError) as excinfo:
        reset_property(None, None)
    assert "Member" in excinfo.exconly()

    with pytest.raises(TypeError) as excinfo:
        prop.reset(None)
    assert "CAtom" in excinfo.exconly()

    with pytest.raises(SystemError) as excinfo:
        prop.reset(Atom())
    assert "invalid member index" in excinfo.exconly()


@pytest.mark.parametrize(
    "mode, func",
    [
        (GetAttr, "set_getattr_mode"),
        (SetAttr, "set_setattr_mode"),
        (DelAttr, "set_delattr_mode"),
    ],
)
def test_property_mode_args_validation(mode, func):
    """Test that a delegator properly validate the arguments when setting mode."""
    with pytest.raises(TypeError) as excinfo:
        getattr(Property(), func)(getattr(mode, "Property"), 1)
    assert "callable or None" in excinfo.exconly()


def test_property_getstate_mode():
    def get_function(obj):
        return obj.i

    def set_function(obj, value):
        obj.i = value

    def del_function(obj):
        del obj.i

    p = Property(get_function)
    assert p.do_should_getstate(Atom()) is False

    p2 = Property(get_function, set_function)
    assert p2.do_should_getstate(Atom()) is True
