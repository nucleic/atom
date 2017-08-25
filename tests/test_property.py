#------------------------------------------------------------------------------
# Copyright (c) 2013-2017, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
"""Test the property and cached property member

"""
import pytest
from atom.api import (Atom, Int, Property, cached_property)


def test_property1():
    """Test defining a property using the arguments.

    """
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
    """Test defining a property using the decorators.

    """
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
    """Test handling missing function(fget, fset, fdel)

    """
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
    """Test using a cached property.

    """
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
    pt.get_member('prop').reset(pt)
    assert pt.prop == 2
