import _io

from typing_extensions import assert_type

from atom.api import Atom, Coerced


# Coerced members validate the same class/instance split as the other descriptors: the
# class exposes the Coerced[...] descriptor, while the instance value remains the concrete
# runtime type after coercion. Each class below exercises a distinct coercion branch.
def g() -> _io.StringIO:
    return _io.StringIO()


class ACoercedScalar(Atom):
    # A scalar coercion target preserves the exact runtime type and keeps the descriptor
    # parameterized with that scalar.
    m = Coerced(int)


class ACoercedTuple(Atom):
    # A tuple of valid runtime types widens the coercion descriptor to the corresponding
    # union while the instance still stores only one concrete type.
    m = Coerced((int, float))


class ACoercedStringIO(Atom):
    # StringIO inputs with a default factory-like initialization should still resolve to the
    # concrete StringIO type on the instance.
    m = Coerced(_io.StringIO, kwargs=dict(initial_value="1"))


class ACoercedFactory(Atom):
    # Factory-based coercion should behave the same as the other direct object coercion
    # scenarios and remain concrete on the instance even though the descriptor is generic.
    m = Coerced(_io.StringIO, factory=g)


assert_type(ACoercedScalar.m, Coerced[int, int])
assert_type(ACoercedScalar().m, int)
assert_type(ACoercedTuple.m, Coerced[int | float, int | float])
assert_type(ACoercedTuple().m, int | float)
assert_type(ACoercedStringIO.m, Coerced[_io.StringIO, _io.StringIO])
assert_type(ACoercedStringIO().m, _io.StringIO)
assert_type(ACoercedFactory.m, Coerced[_io.StringIO, _io.StringIO])
assert_type(ACoercedFactory().m, _io.StringIO)
