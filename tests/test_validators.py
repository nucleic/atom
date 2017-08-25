#------------------------------------------------------------------------------
# Copyright (c) 2013-2017, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
"""Tests for member validation handlers:

    no_op_handler
    bool_handler
    int_handler
    int_promote_handler
    long_handler
    long_promote_handler
    float_handler
    float_promote_handler
    str_handler
    str_promote_handler
    unicode_handler
    unicode_promote_handler
    tuple_handler
    list_handler
    container_list_handler
    dict_handler
    instance_handler
    typed_handler
    subclass_handler
    enum_handler
    callable_handler
    float_range_handler
    range_handler
    coerced_handler
    delegate_handler: not tested here
    object_method_old_new_handler: used when defining validate on Atom subclass
    object_method_name_old_new_handler: unused as far as I can tell
    member_method_object_old_new_handler: used in ForwardType/Instance/Subclass

"""
import sys
import pytest
from future.builtins import int

from atom.api import (CAtom, Atom, Value, Bool, Int, Long, Range, Float,
                      FloatRange, Bytes, Str, Unicode, Enum, Callable, Coerced,
                      Tuple, List, ContainerList, Dict, Instance,
                      ForwardInstance, Typed, ForwardTyped, Subclass,
                      ForwardSubclass, Event)


@pytest.mark.parametrize("member, set_values, values, raising_values",
                         [(Value(), ['a', 1, None], ['a', 1, None], []),
                          (Bool(), [True, False], [True, False], 'r'),
                          (Int(), [1], [1],
                           [1.0, int(1)] if sys.version_info < (3,) else [1.0]
                           ),
                          (Int(strict=False), [1, 1.0, int(1)], 3*[1], ['a']),
                          (Long(strict=True), [int(1)], [int(1)],
                           [1.0, 1] if sys.version_info < (3,) else [0.1]),
                          (Long(strict=False), [1, 1.0, int(1)], 3*[1], ['a']),
                          (Range(0, 2), [0, 2], [0, 2], [-1, 3]),
                          (Range(2, 0), [0, 2], [0, 2], [-1, 3]),
                          (Range(0), [0, 3], [0, 3], [-1]),
                          (Range(high=2), [-1, 2], [-1, 2], [3]),
                          (Float(), [1, int(1), 1.1], [1.0, 1.0, 1.1], ['']),
                          (Float(strict=True), [1.1], [1.1], [1]),
                          (FloatRange(0.0, 0.5), [0.0, 0.5], [0.0, 0.5],
                           [-0.1, 0.6]),
                          (FloatRange(0.5, 0.0), [0.0, 0.5], [0.0, 0.5],
                           [-0.1, 0.6]),
                          (FloatRange(0.0), [0.0, 0.6], [0.0, 0.6], [-0.1]),
                          (FloatRange(high=0.5), [-0.3, 0.5], [-0.3, 0.5],
                           [0.6]),
                          (Bytes(), [b'a', u'a'], [b'a']*2, [1]),
                          (Bytes(strict=True), [b'a'], [b'a'], [u'a']),
                          (Str(), [b'a', u'a'], ['a']*2, [1]),
                          (Str(strict=True),
                           [b'a'] if sys.version_info < (3,) else [u'a'],
                           ['a'],
                           [u'a'] if sys.version_info < (3,) else [b'a']),
                          (Unicode(), [b'a', u'a'], [u'a']*2, [1]),
                          (Unicode(strict=True), [u'a'], [u'a'], [b'a']),
                          (Enum(1, 2, 'a'), [1, 2, 'a'], [1, 2, 'a'], [3]),
                          (Callable(), [int], [int], [1]),
                          (Coerced(set), [{1}, [1], (1,)], [{1}]*3, [1]),
                          (Coerced(int, coercer=lambda x: int(str(x), 2)),
                           ['101'], [5], []),
                          (Tuple(), [(1,)], [(1,)], [[1]]),
                          (Tuple(Int()), [(1,)], [(1,)], [(1.0,)]),
                          (Tuple(int), [(1,)], [(1,)], [(1.0,)]),
                          (List(), [[1]], [[1]], [(1,)]),
                          (List(Int()), [[1]], [[1]], [[1.0]]),
                          (List(float), [[1.0]], [[1.0]], [[1]]),
                          (List((int, float)), [[1, 1.0]], [[1, 1.0]], [['']]),
                          (ContainerList(), [[1]], [[1]], [(1,)]),
                          (ContainerList(Int()), [[1]], [[1]], [[1.0]]),
                          (ContainerList(float), [[1.0]], [[1.0]], [[1]]),
                          (ContainerList((int, float)), [[1, 1.0]], [[1, 1.0]],
                           [['']]),
                          (Dict(), [{1: 2}], [{1: 2}], [()]),
                          (Dict(Int()), [{1: 2}], [{1: 2}], [{'': 2}]),
                          (Dict(value=Int()), [{1: 2}], [{1: 2}], [{2: ''}]),
                          (Dict(int, int), [{1: 2}], [{1: 2}],
                           [{'': 2}, {2: ''}]),
                          (Instance((int, float)), [1, 2.0], [1, 2.0], ['']),
                          (ForwardInstance(lambda: (int, float)),
                           [1, 2.0], [1, 2.0], ['']),
                          (Typed(float), [1.0], [1.0], [1]),
                          (ForwardTyped(lambda: float), [1.0], [1.0], [1]),
                          (Subclass(CAtom), [Atom], [Atom], [int]),
                          (ForwardSubclass(lambda: CAtom),
                           [Atom], [Atom], [int]),
                          ] +
                         ([(Range(sys.maxsize, sys.maxsize + 2),
                           [sys.maxsize, sys.maxsize + 2],
                           [sys.maxsize, sys.maxsize + 2],
                           [sys.maxsize - 1, sys.maxsize + 3])]
                          if sys.version_info > (3,) else [])
                         )
def test_validation_modes(member, set_values, values, raising_values):
    """Test the validation modes.

    """
    class MemberTest(Atom):

        m = member

    tester = MemberTest()
    for sv, v in zip(set_values, values):
        tester.m = sv
        assert tester.m == v

    for rv in raising_values:
        with pytest.raises(ValueError if isinstance(member, Enum)
                           else TypeError):
            tester.m = rv


def test_event_validation():
    """Test validating the payload of an Event.

    """
    class EventValidationTest(Atom):

        ev_member = Event(Int())

        ev_type = Event(int)

    evt = EventValidationTest()

    evt.ev_member = 1
    evt.ev_type = 1
    with pytest.raises(TypeError):
        evt.ev_member = 1.0
    with pytest.raises(TypeError):
        evt.ev_type = 1.0


def test_custom_validate():
    """Test specifying a specific validator in the Atom and using do_validate.

    """
    class ValidatorTest(Atom):

        v = Value(0)

        def _validate_v(self, old, new):
            if not isinstance(new, int):
                raise TypeError()
            if old is not None and new != old + 1:
                raise ValueError()
            return new

    v = ValidatorTest()
    assert v.v == 0
    v.v = 1
    assert v.v == 1
    with pytest.raises(TypeError):
        v.v = None
    assert v.v == 1
    with pytest.raises(ValueError):
        v.v = 4

    v_member = ValidatorTest.v
    with pytest.raises(TypeError):
        v_member.do_validate(v, 1, None)
    assert v.v == 1
    with pytest.raises(ValueError):
        v_member.do_full_validate(v, 1, 4)
