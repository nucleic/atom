from typing import Any, assert_type

from atom.api import Atom, Int, Set


class Z(Atom):
    pass


# Set members follow the same rule as the other container descriptors: the class exposes a
# Set[...] descriptor, while the instance exposes a concrete builtins.set[...] value. Each
# class below isolates one of the main element-inference variants.
class ANoDefault(Atom):
    # No declared element type means the set accepts any element and keeps the descriptor
    # generic.
    m = Set()


class AScalarElement(Atom):
    # A scalar element type narrows the descriptor to Set[int] but preserves a runtime set[int].
    m = Set(int)


class AUnionOfScalarElements(Atom):
    # A tuple of element types widens the descriptor to the union of all accepted types.
    m = Set((int, float, str))


class AMemberBasedSet(Atom):
    # Member-based element validation resolves to the member's concrete runtime type.
    m = Set(Int())


class ADefaultAny(Atom):
    # A default set does not change the generic descriptor type, only the instance value.
    m = Set(default={1})


class ADefaultTyped(Atom):
    # An explicit element type plus a default still narrows the descriptor to the declared
    # set type.
    m = Set(int, default={3})


class ADefaultTupleUnion(Atom):
    # Tuple-based union defaults should widen the descriptor to the union of allowed types
    # while remaining valid for each concrete default item.
    m = Set((int, str), default={1, str(1)})


class ADefaultMixedWithAtom(Atom):
    # Atom subclasses participate in the same union semantics as builtins and are accepted
    # when they appear in a valid default value.
    m = Set((int, str, Z), default={1, str(1), Z()})


assert_type(ANoDefault.m, Set[Any])
assert_type(ANoDefault().m, set[Any])
assert_type(AScalarElement.m, Set[int])
assert_type(AScalarElement().m, set[int])
assert_type(AUnionOfScalarElements.m, Set[int | float | str])
assert_type(AUnionOfScalarElements().m, set[int | float | str])
assert_type(AMemberBasedSet.m, Set[int])
assert_type(AMemberBasedSet().m, set[int])
assert_type(ADefaultAny.m, Set[Any])
assert_type(ADefaultAny().m, set[Any])
assert_type(ADefaultTyped.m, Set[int])
assert_type(ADefaultTyped().m, set[int])
assert_type(ADefaultTupleUnion.m, Set[int | str])
assert_type(ADefaultTupleUnion().m, set[int | str])
assert_type(ADefaultMixedWithAtom.m, Set[int | str | Z])
assert_type(ADefaultMixedWithAtom().m, set[int | str | Z])
assert_type(Set(Int()), Set[int])
