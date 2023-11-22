# --------------------------------------------------------------------------------------
# Copyright (c) 2013-2023, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file LICENSE, distributed with this software.
# --------------------------------------------------------------------------------------
"""Test the notification mechanisms.

"""
import pytest

from atom.api import (
    Atom,
    ChangeType,
    ContainerList,
    Event,
    Int,
    List,
    Signal,
    Value,
    observe,
)


class NonComparableObject:
    def __eq__(self, other):
        raise ValueError()


# --- Static observer manipulations


@pytest.fixture
def static_atom():
    """Class to test static observers."""

    class Extended(Atom):
        val = Int()

    obs_decorator = observe("val2", "ext.val")

    class ObserverTest(Atom):
        cls = Extended

        ext = Value()

        val2 = Int(0)

        changes = List()

        @obs_decorator
        def react(self, change):
            self.changes.append(change["name"])

        manual_obs = obs_decorator(react.func)

    return ObserverTest()


def test_static_observer(static_atom):
    """Test static observers."""
    ot = static_atom

    # Test checking for static observers
    assert ot.get_member("val2").has_observers()
    assert ot.get_member("val2").has_observers(ChangeType.UPDATE)
    assert ot.get_member("val2").has_observer("manual_obs")
    assert ot.get_member("val2").has_observer("react")
    with pytest.raises(TypeError) as excinfo:
        assert ot.get_member("val2").has_observer(1)
    assert "str or callable" in excinfo.exconly()

    # Test notifications on value setting
    ot.val2 = 1
    assert "val2" in ot.changes


def test_manual_static_observers(static_atom):
    """Test manually managing static observers."""

    # Force the use of safe comparison (error cleaning and fallback)
    class Observer:
        def __eq__(self, other):
            raise ValueError()

        def __call__(self, change):
            change["object"].changes.append(change["name"])

    react = Observer()

    # We have 2 static observers hence 2 removals
    member = static_atom.get_member("val2")
    member.remove_static_observer("react")
    member.remove_static_observer("manual_obs")
    assert not member.has_observers()
    static_atom.val2 += 1
    assert not static_atom.changes
    member.add_static_observer(react)
    assert member.has_observer(react)
    static_atom.val2 += 1
    assert static_atom.changes
    member.remove_static_observer(react)
    assert not member.has_observers()

    member.add_static_observer(react, ChangeType.UPDATE)
    assert not member.has_observers(ChangeType.CREATE)
    assert member.has_observers(ChangeType.UPDATE)
    assert member.has_observer(react, ChangeType.UPDATE)
    assert not member.has_observer(react, ChangeType.DELETE)

    with pytest.raises(TypeError) as excinfo:
        member.add_static_observer(1)
    assert "str or callable" in excinfo.exconly()

    with pytest.raises(TypeError) as excinfo:
        member.add_static_observer(react, "foobar")
    assert "int" in excinfo.exconly()

    with pytest.raises(TypeError) as excinfo:
        member.remove_static_observer(1)
    assert "str or callable" in excinfo.exconly()

    # Check errors
    with pytest.raises(TypeError) as excinfo:
        member.has_observer()
    assert "expects a callable" in excinfo.exconly()

    with pytest.raises(TypeError) as excinfo:
        member.has_observer(react, 1, True)
    assert "expects a callable" in excinfo.exconly()

    with pytest.raises(TypeError) as excinfo:
        member.has_observer(react, "bool")
    assert "int" in excinfo.exconly()


@pytest.mark.parametrize(
    "change_type, expected_types",
    [
        (ChangeType.ANY, ["create", "update", "delete"]),
        (ChangeType.CREATE, ["create"]),
        (ChangeType.UPDATE, ["update"]),
        (ChangeType.DELETE, ["delete"]),
        (ChangeType.UPDATE | ChangeType.DELETE, ["update", "delete"]),
        (0, []),
        (100000, []),
    ],
)
def test_static_observers_change_types(change_type, expected_types):
    """Test manually managing static observers."""

    # Force the use of safe comparison (error cleaning and fallback)
    class Widget(Atom):
        val = Value()

    changes = []

    def react(change):
        changes.append(change)

    Widget.val.add_static_observer(react, change_type)
    w = Widget()
    w.val
    w.val = 1
    del w.val
    assert len(changes) == len(expected_types)
    for change, exp in zip(changes, expected_types):
        assert change["type"] == exp


def test_modifying_static_observers_in_callback():
    """Test modifying the static observers in an observer."""

    class ChangingAtom(Atom):
        val = Int()

        counter1 = Int()

        counter2 = Int()

        @observe("val")
        def react1(self, change):
            self.counter1 += 1
            m = self.get_member("val")
            m.remove_static_observer("react1")
            m.add_static_observer("react2")

        def react2(self, change):
            self.counter2 += 1
            m = self.get_member("val")
            m.remove_static_observer("react2")
            m.add_static_observer("react1")

    ca = ChangingAtom()
    assert ChangingAtom.val.has_observer("react1")
    assert not ChangingAtom.val.has_observer("react2")
    ca.val = 1
    assert ca.counter1 == 1
    # Ensure the modification take place after notification dispatch is
    # complete
    assert ca.counter2 == 0
    assert ChangingAtom.val.has_observer("react2")
    assert not ChangingAtom.val.has_observer("react1")

    ca.val += 1
    assert ca.counter2 == 1
    # Ensure the modification take place after notification dispatch is
    # complete
    assert ca.counter1 == 1
    assert ChangingAtom.val.has_observer("react1")
    assert not ChangingAtom.val.has_observer("react2")

    # Test handling exception in the guard map that ensure that the
    # modifications to the observers occur after the notification dispatch
    def raising_observer(change):
        raise ValueError()

    ChangingAtom.val.add_static_observer(raising_observer)

    with pytest.raises(ValueError):
        ca.val += 1

    assert ca.counter1 == 2
    # Ensure the modification take place after notification dispatch is
    # complete
    assert ca.counter2 == 1
    assert ChangingAtom.val.has_observer("react2")


def test_copy_static_observers(static_atom):
    """Test cloning the static observers of a member."""
    member = static_atom.get_member("val2")
    v = Value()
    v.copy_static_observers(member)
    assert v.has_observers()
    assert v.has_observer("manual_obs")
    assert v.has_observer("react")

    # This is ano-op and take an early exit seen in coverage.
    v.copy_static_observers(v)

    with pytest.raises(TypeError) as excinfo:
        v.copy_static_observers(1)
    assert "Member" in excinfo.exconly()


def test_extended_static_observers(static_atom):
    """Test using extended static observers."""
    ot = static_atom
    ext1 = static_atom.cls()
    ext2 = static_atom.cls()

    # Test installing the extended observer
    ot.ext = ext1
    assert ext1.has_observer("val", ot.react)
    assert not ext2.has_observer("val", ot.react)

    ot.ext = ext2
    assert ext2.has_observer("val", ot.react)
    assert not ext1.has_observer("val", ot.react)

    # Test notifications on value setting
    ot.val2 = 1
    assert "val2" in ot.changes
    ext1.val = 1
    assert "val" not in ot.changes
    ext2.val = 1
    assert "val" in ot.changes

    # Test removing the extended observer upon deletion
    del ot.ext
    assert not ext2.has_observer("val", ot.react)

    with pytest.raises(TypeError):
        ot.ext = 12


def test_observe_decorators():
    """Test checking observe decorator handling."""

    def react(self, change):
        pass

    handler = observe(("val",))
    handler(react)
    handler_clone = handler.clone()
    assert handler is not handler_clone
    assert handler.pairs == handler_clone.pairs
    assert handler.func is handler_clone.func

    with pytest.raises(TypeError):
        observe(12)
    with pytest.raises(TypeError):
        observe(["a.b.c"])


# --- Dynamic observer manipulations


class Observer(object):
    """Observer used for testing dynamic observers."""

    def __init__(self):
        self.count = 0

    def react(self, change):
        self.count += 1

    # Force the use of safe equality comparison (ie clean error and fallback
    # on pointer comparison)
    def __eq__(self, other):
        raise ValueError()


class DynamicAtom(Atom):
    """Atom used to test dynamic observers."""

    val = Int()
    val2 = Int()
    val3 = Int()


def test_single_observe():
    """Test observing a single member from a single instance."""
    dt1 = DynamicAtom()
    dt2 = DynamicAtom()
    observer = Observer()

    dt1.observe("val", observer.react)
    # Test creation in the absence of static observers
    dt1.val
    assert observer.count == 1
    dt2.val = 1
    assert observer.count == 1
    del dt1.val
    assert observer.count == 2


def test_multiple_observe():
    """Test observing multiple members from a single instance."""
    dt1 = DynamicAtom()
    observer = Observer()

    dt1.observe(("val", "val2"), observer.react)
    dt1.val = 1
    assert observer.count == 1
    dt1.val2 = 1
    assert observer.count == 2


def test_observe_change_types():
    """Test observing multiple members from a single instance."""
    dt1 = DynamicAtom()

    changes = []

    def on_change(change):
        changes.append(change)

    dt1.observe("val", on_change, ChangeType.UPDATE | ChangeType.DELETE)
    dt1.val = 1
    assert len(changes) == 0
    dt1.val = 2
    assert len(changes) == 1
    assert changes[0]["type"] == "update"
    del dt1.val
    assert len(changes) == 2
    assert changes[1]["type"] == "delete"


def test_wrong_args_observe():
    """Test handling of wrong arguments to observe."""
    dt1 = DynamicAtom()

    with pytest.raises(TypeError) as excinfo:
        dt1.observe("val")
    assert "2 or 3 arguments" in excinfo.exconly()

    with pytest.raises(TypeError) as excinfo:
        dt1.observe("val", lambda change: change, ChangeType.ANY, "bar")
    assert "2 or 3 arguments" in excinfo.exconly()

    with pytest.raises(TypeError) as excinfo:
        dt1.observe(1, lambda change: change)
    assert "iterable" in excinfo.exconly()

    with pytest.raises(TypeError) as excinfo:
        dt1.observe((1, 1), lambda change: change)
    assert "str" in excinfo.exconly()

    with pytest.raises(TypeError) as excinfo:
        dt1.observe("val", 1)
    assert "callable" in excinfo.exconly()

    with pytest.raises(TypeError) as excinfo:
        dt1.observe("val", lambda change: change, "foo")
    assert "int" in excinfo.exconly()


@pytest.fixture()
def observed_atom():
    """Observed atom used to test unobserve."""
    a = DynamicAtom()
    ob1 = Observer()
    ob2 = Observer()
    a.observe(("val", "val2", "val3"), ob1.react)
    a.observe(("val", "val2", "val3"), ob2.react)
    return a, ob1, ob2


def test_unobserving_no_args(observed_atom):
    """Test removing all observers at once."""
    a, _, _ = observed_atom
    a.unobserve()
    for m in a.members():
        assert not a.has_observers(m)


def test_unobserving_a_single_member(observed_atom):
    """Test removing the observers from a single member."""
    a, _, _ = observed_atom
    assert a.has_observers("val")
    assert a.has_observers("val2")
    a.unobserve("val")
    assert not a.has_observers("val")
    assert a.has_observers("val2")


def test_unobserving_multiple_members(observed_atom):
    """Test removing observers from multiple members."""
    a, _, _ = observed_atom
    assert a.has_observers("val")
    assert a.has_observers("val2")
    a.unobserve(("val", "val2"))
    assert not a.has_observers("val")
    assert not a.has_observers("val2")
    assert a.has_observers("val3")


def test_removing_specific_observer(observed_atom):
    """Test removing a specific observer from a member."""
    a, ob1, ob2 = observed_atom
    assert a.has_observer("val", ob1.react)
    assert a.has_observer("val", ob2.react)
    a.unobserve("val", ob2.react)
    assert a.has_observer("val", ob1.react)
    assert not a.has_observer("val", ob2.react)


def test_removing_specific_observer2(observed_atom):
    """Test removing a specific observer from multiple members."""
    a, ob1, ob2 = observed_atom
    for m in ("val", "val2", "val3"):
        assert a.has_observer(m, ob1.react)
        assert a.has_observer(m, ob2.react)
    a.unobserve(("val", "val2"), ob2.react)
    for m in ("val", "val2"):
        assert a.has_observer(m, ob1.react)
        assert not a.has_observer("val", ob2.react)
    assert a.has_observer("val3", ob1.react)
    assert a.has_observer("val3", ob2.react)


def test_wrong_args_unobserve(observed_atom):
    """Test handling of bad arguments to unobserve."""
    a, _, _ = observed_atom

    # Too many args
    with pytest.raises(TypeError) as excinfo:
        a.unobserve("val", lambda change: change, 1)
    assert "2 arguments" in excinfo.exconly()

    # Non-iterable first arg
    with pytest.raises(TypeError) as excinfo:
        a.unobserve(1)
    assert "iterable" in excinfo.exconly()

    # Non-iterable first arg with callable
    with pytest.raises(TypeError) as excinfo:
        a.unobserve(1, lambda change: change)
    assert "iterable" in excinfo.exconly()

    # Non iterable fo string first arg
    with pytest.raises(TypeError) as excinfo:
        a.unobserve((1, 1))
    assert "str" in excinfo.exconly()

    # Non iterable fo string first arg with callable
    with pytest.raises(TypeError) as excinfo:
        a.unobserve((1, 1), lambda change: change)
    assert "str" in excinfo.exconly()

    # Non callable second arg
    with pytest.raises(TypeError) as excinfo:
        a.unobserve("vsl", 1)
    assert "callable" in excinfo.exconly()


def test_wrong_arg_to_has_observer(observed_atom):
    """Test non string arg to has_observer."""
    a, _, _ = observed_atom
    with pytest.raises(TypeError) as excinfo:
        a.has_observer(1, lambda x: x, 1)
    assert "2 arguments" in excinfo.exconly()

    with pytest.raises(TypeError) as excinfo:
        a.has_observer(1, lambda x: x)
    assert "str" in excinfo.exconly()

    with pytest.raises(TypeError) as excinfo:
        a.has_observer("val", 1)
    assert "callable" in excinfo.exconly()


def test_binding_event_signals():
    """Test directly binding events and signals."""

    class EventSignalTest(Atom):
        e = Event()

        s = Signal()

        counter = Int()

    def event_handler(change):
        change["object"].counter += 1

    def signal_handler(obj):
        obj.counter += 2

    est = EventSignalTest()
    est.e.bind(event_handler)
    est.s.connect(signal_handler)

    est.e = 1
    est.s(est)
    assert est.counter == 3
    est.e(1)
    est.s.emit(est)
    assert est.counter == 6

    est.e.unbind(event_handler)
    est.s.disconnect(signal_handler)


def test_modifying_dynamic_observers_in_callback():
    """Test modifying the static observers in an observer."""

    class InvalidObserver(object):
        """Silly callable which always evaluate to false."""

        def __init__(self, active):
            self.active = active

        def __bool__(self):
            return self.active

        __nonzero__ = __bool__

        def __call__(self, change):
            pass

    class ChangingAtom(Atom):
        val = Int()

        counter1 = Int()

        counter2 = Int()

        observer = Value()

        def react1(self, change):
            self.counter1 += 1
            self.observer.active = False
            self.unobserve("val", self.react1)
            self.observe("val", self.react2)

        def react2(self, change):
            self.counter2 += 1
            self.unobserve("val")
            self.observe("val", self.react1)

    ca = ChangingAtom()
    ca.observer = invalid_obs = InvalidObserver(True)
    ca.observe("val", invalid_obs)
    ca.observe("val", ca.react1)
    assert ca.has_observer("val", ca.react1)
    assert not ca.has_observer("val", ca.react2)
    ca.val = 1
    assert ca.counter1 == 1
    # Ensure the modification take place after notification dispatch is
    # complete
    assert ca.counter2 == 0
    assert ca.has_observer("val", ca.react2)
    assert not ca.has_observer("val", ca.react1)
    assert not ca.has_observer("val", invalid_obs)

    ca.val += 1
    assert ca.counter2 == 1
    # Ensure the modification take place after notification dispatch is
    # complete
    assert ca.counter1 == 1
    assert ca.has_observer("val", ca.react1)
    assert not ca.has_observer("val", ca.react2)

    # Test handling exception in the guard map that ensure that the
    # modifications to the observers occur after the notification dispatch
    def raising_observer(change):
        raise ValueError()

    ca.observe("val", raising_observer)

    with pytest.raises(ValueError):
        ca.val += 1

    assert ca.counter1 == 2
    # Ensure the modification take place after notification dispatch is
    # complete
    assert ca.counter2 == 1
    assert ca.has_observer("val", ca.react2)


# --- Notifications generation and handling


@pytest.fixture
def sd_observed_atom():
    """Atom object with both static and dynamic observers."""

    class Observer(object):
        def __init__(self):
            self.count = 0

        def react(self, change):
            print(change)
            self.count += 1

    class NotifTest(Atom):
        val = Value()

        count = Int()

        observer = Value()

        def __init__(self):
            super(NotifTest, self).__init__()
            self.observer = Observer()
            self.observe("val", self.observer.react)

        def _observe_val(self, change):
            print(change)
            self.count += 1

    return NotifTest()


def test_notification_on_creation_deletion(sd_observed_atom):
    """Test that observers are properly called on creation."""
    # Create value based on default
    sd_observed_atom.val

    assert sd_observed_atom.count == 1
    assert sd_observed_atom.observer.count == 1

    del sd_observed_atom.val

    assert sd_observed_atom.count == 2
    assert sd_observed_atom.observer.count == 2

    sd_observed_atom.val

    assert sd_observed_atom.count == 3
    assert sd_observed_atom.observer.count == 3


def test_notification_on_setting(sd_observed_atom):
    """Test that notifiers are called when setting a value."""
    sd_observed_atom.val = 1

    assert sd_observed_atom.count == 1
    assert sd_observed_atom.observer.count == 1

    # And also test update of values
    sd_observed_atom.val = 2

    assert sd_observed_atom.count == 2
    assert sd_observed_atom.observer.count == 2


def test_notification_on_setting_non_comparable_value(sd_observed_atom):
    """Test that notifiers are called when setting a value."""
    o1 = NonComparableObject()
    sd_observed_atom.val = 0

    assert sd_observed_atom.count == 1
    assert sd_observed_atom.observer.count == 1

    # And also test update of values
    sd_observed_atom.val = 1

    assert sd_observed_atom.count == 2
    assert sd_observed_atom.observer.count == 2

    # No notification on equal assignment
    sd_observed_atom.val = 1

    assert sd_observed_atom.count == 2
    assert sd_observed_atom.observer.count == 2

    # Check notification on invalid comparison
    sd_observed_atom.val = o1

    assert sd_observed_atom.count == 3
    assert sd_observed_atom.observer.count == 3

    # Check no notification on equal value assignment
    sd_observed_atom.val = o1

    assert sd_observed_atom.count == 3
    assert sd_observed_atom.observer.count == 3


def test_enabling_disabling_notifications(sd_observed_atom):
    """Test enabling/disabling notification on an atom."""
    assert sd_observed_atom.notifications_enabled()
    sd_observed_atom.val = 1
    assert sd_observed_atom.count == 1
    assert sd_observed_atom.observer.count == 1
    sd_observed_atom.set_notifications_enabled(False)
    sd_observed_atom.val = 0
    assert sd_observed_atom.count == 1
    assert sd_observed_atom.observer.count == 1

    with pytest.raises(TypeError):
        sd_observed_atom.set_notifications_enabled("")

    sd_observed_atom.__sizeof__()  # check that observers do not cause issues


def test_manually_notifying(sd_observed_atom):
    """Test manual notifications"""
    nt = sd_observed_atom
    ob = nt.observer

    # Check both static and dynamic notifiers are called
    nt.val = 1
    assert ob.count == 1
    assert nt.count == 1

    # Check only dynamic notifiers are called
    nt.notify("val", {"name": "val"})
    assert ob.count == 2
    assert nt.count == 1

    # Check that only static notifiers are called
    type(nt).val.notify(nt, {})
    assert ob.count == 2
    assert nt.count == 2

    # Check that notification suppression does work
    ob.count = 0
    nt.count = 0
    with nt.suppress_notifications():
        nt.val += 1
    assert not nt.count and not ob.count

    # Check bad argument
    with pytest.raises(TypeError) as excinfo:
        nt.notify()
    assert "1 argument" in excinfo.exconly()

    with pytest.raises(TypeError) as excinfo:
        nt.notify(1)
    assert "str" in excinfo.exconly()

    with pytest.raises(TypeError) as excinfo:
        type(nt).val.notify(1)
    assert "CAtom" in excinfo.exconly()


def test_event_notification(sd_observed_atom):
    """Check that Event do call both static and dynamic observers."""

    class EventAtom(type(sd_observed_atom)):
        val = Event()

        def _observe_val(self, change):
            self.count += 1

    ea = EventAtom()
    ea.val = 1

    assert ea.count == 1
    assert ea.observer.count == 1


def test_signal_notification(sd_observed_atom):
    """Check that Signal do call both static and dynamic observers."""

    class SignalAtom(type(sd_observed_atom)):
        val = Signal()

        def _observe_val(self, change):
            self.count += 1

    sa = SignalAtom()
    sa.val(1)

    assert sa.count == 1
    assert sa.observer.count == 1


def test_static_observer_container_change_type():
    """Test observing a single member from a single instance."""

    class Widget(Atom):
        items = ContainerList()

    changes = []

    def react(change):
        changes.append(change)

    Widget.items.add_static_observer(react, ChangeType.CREATE | ChangeType.UPDATE)

    w = Widget()
    w.items = []
    assert len(changes) == 1
    assert changes[0]["type"] == "create"
    changes.clear()

    w.items.append(1)
    assert len(changes) == 0  # Container ignored

    Widget.items.add_static_observer(react, ChangeType.CONTAINER)
    w.items.append(1)
    assert len(changes) == 1
    assert changes[0]["type"] == "container"
    changes.clear()

    w.items = []
    assert len(changes) == 0

    Widget.items.add_static_observer(react, ChangeType.UPDATE | ChangeType.CONTAINER)
    w.items = [1, 2]
    assert len(changes) == 1
    assert changes[0]["type"] == "update"


def test_observe_decorator_change_type():
    """Test observing a single member from a single instance."""

    changes = []

    change_types = ChangeType.ANY & ~(ChangeType.CREATE)

    class Widget(Atom):
        items = ContainerList()

        @observe("items", change_types=change_types)
        def _on_items_change(self, change):
            changes.append(change)

    w = Widget()
    w.items
    assert len(changes) == 0
    w.items.append(1)
    assert len(changes) == 1
    assert changes[0]["type"] == "container"
    changes.clear()
    del w.items
    assert len(changes) == 1
    assert changes[0]["type"] == "delete"
    changes.clear()

    w.items  # Create default
    assert len(changes) == 0
    w.items = [2]  # Update
    assert len(changes) == 1
    assert changes[0]["type"] == "update"
