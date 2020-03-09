#------------------------------------------------------------------------------
# Copyright (c) 2013-2017, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
#------------------------------------------------------------------------------
"""Tests the intenum implementation.

"""
import sys
from pickle import dumps, loads
from operator import (xor, and_, or_, invert, add, sub, mul, truediv,
                      floordiv, mod, lshift, rshift, neg, pos)
if sys.version_info < (3,):
    from operator import div

import pytest
from atom.intenum import IntEnum, _IntEnumMeta


INVALID_BINARY_OPS = (add, sub, mul, truediv, floordiv, mod, divmod, pow,
                      lshift, rshift)
if sys.version_info < (3,):
    INVALID_BINARY_OPS = INVALID_BINARY_OPS + (div,)

INVALID_UNARY_OP = (neg, pos, abs)


def test_metaclass_safeties():
    """Test the safeties of the metaclass.

    """
    with pytest.raises(TypeError):
        class FalseIntEnum(object, metaclass=_IntEnumMeta):
            pass

    with pytest.raises(TypeError):
        class TwoManyBases(int, object, metaclass=_IntEnumMeta):
            pass


class EnumTest(IntEnum):

        a = 1
        b = 2


def test_int_enum():
    """Test the IntEnum class.

    """
    # Test len
    assert len(EnumTest) == 2

    # Test contains
    assert 'a' in EnumTest
    assert 2 in EnumTest

    # Test iter
    assert sorted(iter(EnumTest)) == [EnumTest.a, EnumTest.b]

    # Test enum values access
    assert EnumTest.a is EnumTest('a')
    assert EnumTest.b is EnumTest[2]
    with pytest.raises(TypeError):
        EnumTest(None)
    with pytest.raises(ValueError):
        EnumTest('c')
    with pytest.raises(TypeError):
        EnumTest.a = 2

    assert EnumTest.a.name == 'a'

    # Test repr and str
    assert '<enum' in repr(EnumTest.a)
    assert str(EnumTest.a) == 'EnumTest.a'


def test_enum_flags():
    """Test using the enum flags.

    """
    # Test using flags instances
    flags_cls = EnumTest.Flags
    assert EnumTest.Flags is flags_cls
    assert flags_cls(0) == 0
    flag = flags_cls('a')
    assert flag == 1
    assert flag is flags_cls(flag)
    assert 'enumflags' in repr(flag)
    assert str(flag) == 'EnumTestFlags'

    # Test logic operations
    assert and_(flag, EnumTest.a)
    assert or_(flags_cls('b'), flag)
    assert xor(flags_cls('b'), flag) == 3
    with pytest.raises(TypeError):
        and_(flag, 2)
    assert invert(flag) == -2


@pytest.mark.parametrize("obj, op, other, res",
                         [(EnumTest.a, and_, EnumTest.a, 1),
                          (EnumTest.a, or_, EnumTest.Flags('a'), 1),
                          (EnumTest.a, xor, EnumTest.Flags('b'), 3),
                          (EnumTest.a, invert, None, -2),
                          (EnumTest.Flags('a'),  and_, EnumTest.Flags('a'), 1),
                          (EnumTest.Flags('a'), or_, EnumTest.a,  1),
                          (EnumTest.Flags('a'), xor, EnumTest.b, 3),
                          (EnumTest.Flags('a'), invert, None, -2)])
def test_valid_operations(obj, op, other, res):
    """Test the valid operations for enum and flags.

    """
    if other is not None:
        assert op(obj, other) == res
        assert op(other, obj) == res
        with pytest.raises(TypeError):
            op(obj, 2)
        with pytest.raises(TypeError):
            op(2, obj)
    else:
        assert op(obj) == res


@pytest.mark.parametrize("obj, op",
                         [(o, op) for op in INVALID_BINARY_OPS
                          for o in [EnumTest.a, EnumTest.Flags('a')]])
def test_invalid_binary_operators(obj, op):
    """Test invalid binary operators for enum and flags.

    """
    with pytest.raises(TypeError):
        op(obj, 1)
    # Test r-operator
    with pytest.raises(TypeError):
        op(1, obj)


@pytest.mark.parametrize("obj, op",
                         [(o, op) for op in INVALID_UNARY_OP
                          for o in [EnumTest.a, EnumTest.Flags('a')]])
def test_invalid_unary_operators(obj, op):
    """Test invalid binary operators for enum and flags.

    """
    with pytest.raises(TypeError):
        op(obj)


def test_pickling():
    """Test pickling and unpickling enum and flags.

    """
    d = dumps(EnumTest.a)
    assert loads(d) == EnumTest.a

    d = dumps(EnumTest.Flags('a'))
    assert loads(d) == EnumTest.Flags('a')
