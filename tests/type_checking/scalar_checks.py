from typing import Any, assert_type

from atom.api import (
    Atom,
    Bool,
    Bytes,
    Constant,
    Float,
    FloatRange,
    Int,
    Range,
    ReadOnly,
    Str,
    Value,
)


def g() -> int:
    return 1


# Scalar member checks focus on the descriptor/value split: the class receives the scalar
# member descriptor while the instance exposes the concrete Python value. Each class below
# covers a distinct scalar validation path.
class AValue(Atom):
    # A plain Value member accepts any runtime value and keeps the descriptor generic.
    m = Value()


class AReadOnlyDefault(Atom):
    # ReadOnly accepts a union of allowed types and should keep that union on the class
    # while instantiating a concrete scalar value on the instance.
    m = ReadOnly((int, str), default=0)


class AReadOnlyFactory(Atom):
    # A factory default is semantically equivalent to the same allowed union, but derived
    # through the factory path rather than a literal default.
    m = ReadOnly((int, str), factory=g)


class AConstantKindUnion(Atom):
    # Constant kinds behave like a fixed allowed-union descriptor, preserving the allowed
    # scalar types on the class.
    m = Constant(kind=(int, str))


class AIntLoose(Atom):
    # Int(strict=False) accepts both int and float values, so the descriptor widens to the
    # union while the runtime instance value remains the concrete int when it is used.
    m = Int(strict=False)


class AFloatRangeDefault(Atom):
    # FloatRange keeps the descriptor as a floating-range value while the instance value is
    # the concrete float object.
    m = FloatRange()


class ARangeDefault(Atom):
    # Range is a constrained integer-like scalar and should narrow the descriptor to int.
    m = Range()


class AFloatStrict(Atom):
    # The strict float descriptor keeps the exact float type on the class and float on the
    # instance.
    m = Float(strict=True)


class ABytesLoose(Atom):
    # Bytes(strict=False) accepts both bytes and str values while the value at runtime is
    # the concrete bytes object.
    m = Bytes(strict=False)


class AStrLoose(Atom):
    # Str(strict=False) accepts both str and bytes at validation time while the runtime
    # instance value remains the concrete str object.
    m = Str(strict=False)


class ABool(Atom):
    # Bool members are the canonical boolean scalar case and should remain bool on both the
    # descriptor and the instance value.
    m = Bool()


assert_type(AValue.m, Value[Any])
assert_type(AValue().m, Any)
assert_type(AReadOnlyDefault.m, ReadOnly[int | str])
assert_type(AReadOnlyDefault().m, int | str)
assert_type(AReadOnlyFactory.m, ReadOnly[int | str])
assert_type(AReadOnlyFactory().m, int | str)
assert_type(AConstantKindUnion.m, Constant[int | str])
assert_type(AConstantKindUnion().m, int | str)
assert_type(AIntLoose.m, Int[int | float])
assert_type(AIntLoose().m, int)
assert_type(AFloatRangeDefault.m, FloatRange[int | float])
assert_type(AFloatRangeDefault().m, float)
assert_type(ARangeDefault.m, Range[int])
assert_type(ARangeDefault().m, int)
assert_type(AFloatStrict.m, Float[float])
assert_type(AFloatStrict().m, float)
assert_type(ABytesLoose.m, Bytes[bytes | str])
assert_type(ABytesLoose().m, bytes)
assert_type(AStrLoose.m, Str[str | bytes])
assert_type(AStrLoose().m, str)
assert_type(ABool.m, Bool[bool])
assert_type(ABool().m, bool)
