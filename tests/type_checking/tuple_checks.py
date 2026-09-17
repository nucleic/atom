from typing import Any

from typing_extensions import assert_type

from atom.api import Atom, FixedTuple, Int, Tuple


# Tuple members keep the descriptor on the class and a concrete tuple on the instance.
# The classes below cover the main inference variants for dynamic tuples and fixed-length
# tuples without relying on generated fixture metadata.
class ANoDefault(Atom):
    # An untyped tuple member accepts any tuple element and leaves the descriptor generic.
    m = Tuple()


class AScalarTuple(Atom):
    # A scalar type narrows tuple element inference while the instance still stores a real
    # tuple of the matching element type.
    m = Tuple(int)


class AUnionTuple(Atom):
    # Tuple-based unions widen the element type to the union of all accepted values.
    m = Tuple((int, float, str))


class AMemberTuple(Atom):
    # A member value should resolve to the same concrete type as the member itself.
    m = Tuple(Int())


class ADefaultAny(Atom):
    # Any default tuple should not force the descriptor to a narrower type when no element
    # type is declared.
    m = Tuple(default=(1,))


class ADefaultTyped(Atom):
    # An explicit element type plus a default keeps the descriptor narrowed and preserves
    # the instance tuple type.
    m = Tuple(int, default=(3,))


class ADefaultUnion(Atom):
    # Defaults that contain valid instances from multiple tuple members should widen to the
    # full union accepted by the descriptor.
    m = Tuple((int, str), default=(1, str(1)))


class ADefaultBytesUnion(Atom):
    # Three-way unions follow the same inference pattern as two-way unions, with bytes
    # included in the accepted type set.
    m = Tuple((int, str, bytes), default=(1, str(1), bytes(1)))


class AFixed(Atom):
    # FixedTuple records the exact element shape on the descriptor and the concrete tuple
    # type on the instance.
    m = FixedTuple(int, float)


class AFixedDefault(Atom):
    # A fixed tuple with a default still exposes the exact fixed-length tuple descriptor and
    # the runtime tuple value type.
    m = FixedTuple(int, default=(3,))


assert_type(ANoDefault.m, Tuple[Any])
assert_type(ANoDefault().m, tuple[Any, ...])
assert_type(AScalarTuple.m, Tuple[int])
assert_type(AScalarTuple().m, tuple[int, ...])
assert_type(AUnionTuple.m, Tuple[int | float | str])
assert_type(AUnionTuple().m, tuple[int | float | str, ...])
assert_type(AMemberTuple.m, Tuple[int])
assert_type(AMemberTuple().m, tuple[int, ...])
assert_type(ADefaultAny.m, Tuple[Any])
assert_type(ADefaultAny().m, tuple[Any, ...])
assert_type(ADefaultTyped.m, Tuple[int])
assert_type(ADefaultTyped().m, tuple[int, ...])
assert_type(ADefaultUnion.m, Tuple[int | str])
assert_type(ADefaultUnion().m, tuple[int | str, ...])
assert_type(ADefaultBytesUnion.m, Tuple[int | str | bytes])
assert_type(ADefaultBytesUnion().m, tuple[int | str | bytes, ...])
assert_type(AFixed.m, FixedTuple[tuple[int, float]])
assert_type(AFixed().m, tuple[int, float])
assert_type(AFixedDefault.m, FixedTuple[tuple[int]])
assert_type(AFixedDefault().m, tuple[int])
assert_type(Tuple(Int()), Tuple[int])
