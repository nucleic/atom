#------------------------------------------------------------------------------
# Copyright (c) 2013-2017, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
"""Test the notification mechanisms.

"""
import pytest
from atom.api import Atom, Int, List, Value, observe


def test_static_observers():
    """Test using static observers.

    """
    class Extended(Atom):

        val = Int()

    obs_decorator = observe('val2', 'ext.val')

    class ObserverTest(Atom):

        ext = Value()

        val2 = Int(0)

        changes = List()

        @obs_decorator
        def react(self, change):
            self.changes.append(change['name'])

        manual_obs = obs_decorator(react.func)

    ot = ObserverTest()
    ext1 = Extended()
    ext2 = Extended()

    # Test installing the extended observer
    ot.ext = ext1
    assert ext1.has_observer('val', ot.react)
    assert not ext2.has_observer('val', ot.react)

    ot.ext = ext2
    assert ext2.has_observer('val', ot.react)
    assert not ext1.has_observer('val', ot.react)

    # Test notifications on value setting
    ot.val2 = 1
    assert 'val2' in ot.changes
    ext1.val = 1
    assert 'val' not in ot.changes
    ext2.val = 1
    assert 'val' in ot.changes

    # Test manually managing static observers
    ot.changes = []
    # We have 2 static observers hence 2 removals
    ObserverTest.val2.remove_static_observer('react')
    ObserverTest.val2.remove_static_observer('manual_obs')
    ot.val2 += 1
    assert not ot.changes
    ObserverTest.val2.add_static_observer('react')
    ot.val2 += 1
    assert ot.changes

    # Test removing the extended observer upon deletion
    del ot.ext
    assert not ext2.has_observer('val', ot.react)

    with pytest.raises(TypeError):
        ot.ext = 12


def test_dynamic_observers():
    """Test using dynamic observers.

    """
    class Observer(object):

        def __init__(self):
            self.count = 0

        def react(self, change):
            self.count += 1

    class DynamicTest(Atom):

        val = Int()

        val2 = Int()

    ob = Observer()
    dt1 = DynamicTest()
    dt2 = DynamicTest()

    # Test observing a single instance
    dt1.observe('val', ob.react)
    dt1.val = 1
    assert ob.count == 1
    dt2.val = 1
    assert ob.count == 1

    # Test unobserving (no args)
    dt1.observe('val2', ob.react)
    dt1.unobserve()
    for m in dt1.members():
        assert not dt1.has_observers(m)

    # Test unobserving (single arg)
    dt1.observe('val', ob.react)
    dt1.observe('val2', ob.react)
    assert dt1.has_observers('val')
    assert dt1.has_observers('val2')
    dt1.unobserve('val')
    assert not dt1.has_observers('val')
    assert dt1.has_observers('val2')

    # Test unobserving (two args)
    ob2 = Observer()
    dt2.observe('val', ob.react)
    dt2.observe('val', ob2.react)
    assert dt2.has_observer('val', ob.react)
    assert dt2.has_observer('val', ob2.react)
    dt2.unobserve('val', ob2.react)
    assert dt2.has_observer('val', ob.react)
    assert not dt2.has_observer('val', ob2.react)


def test_manually_notifying():
    """Test manual notifications

    """
    class Observer(object):

        def __init__(self):
            self.count = 0

        def react(self, change):
            self.count += 1

    class NotifTest(Atom):

        val = Int()

        count = Int()

        def _observe_val(self, change):
            self.count += 1

    ob = Observer()
    nt = NotifTest()
    nt.observe('val', ob.react)

    # Check both static and dynamic notifiers are called
    nt.val = 1
    assert ob.count == 1
    assert nt.count == 1

    # Check only dynamic notifiers are called
    nt.notify('val', dict(name='val'))
    assert ob.count == 2
    assert nt.count == 1

    # Check that only static notifiers are called
    NotifTest.val.notify(nt, dict())
    assert ob.count == 2
    assert nt.count == 2

    # Check that notification suppression does work
    ob.count = 0
    nt.count = 0
    with nt.suppress_notifications():
        nt.val += 1
    assert not nt.count and not ob.count


def test_observe_decorators():
    """Test checking observe decorator handling.

    """
    def react(self, change):
        pass
    handler = observe(('val',))
    handler(react)
    handler_clone = handler.clone()
    assert handler is not handler_clone
    assert handler.pairs == handler_clone.pairs
    assert handler.func is handler_clone.func

    with pytest.raises(TypeError):
        observe(12)
    with pytest.raises(TypeError):
        observe(['a.b.c'])

# xxx test connecting from the value return by __get__ of Signal/Event
# XXX add a test catching the SystemError of Python 3
